/**
 ******************************************************************************
 * @file    WZTOE/WZTOE_DHCPClient/main.c
 * @author  WIZnet
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2018 WIZnet</center></h2>
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "main.h"
#include "wizchip_conf.h"
#include "dhcp.h"
#include "i2c.h"
#include "ssd1306.h"
#include "max30102.h"
#include "pulse.h"
#include "waveform.h"
#include "httpServer.h"
#include "httpParser.h"
#include "webpage.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;
static __IO uint32_t uwTick;

wiz_NetInfo gWIZNETINFO;

#define DATA_BUF_SIZE 2048
uint8_t RX_BUF[DATA_BUF_SIZE];
uint8_t TX_BUF[DATA_BUF_SIZE];

#define NTP_BUF_SIZE 48
uint8_t ntp_buf[NTP_BUF_SIZE];

#define PUSH_BUF_SIZE 256
uint8_t push_rcv_buf[PUSH_BUF_SIZE];
uint8_t push_buf[PUSH_BUF_SIZE];
uint8_t push_body_buf[PUSH_BUF_SIZE];

#define DHCP_SOCKET_NUM     0
#define SNTP_SOCKET_NUM     1
#define PUSH_SOCKET_NUM     2

#define MAX_HTTPSOCK	4
uint8_t socknumlist[] = { 3, 4, 5, 6};

int32_t peak = 0;
int32_t beatAvg = 0;
int32_t SPO2 = 0;
int32_t SPO2f = 0;

waveform_type* p_wave = NULL;

#define SNTPCLK 64
static uint8_t ntp_server[4] = {121, 174, 142, 82};
volatile uint32_t ntp_timer_count = 0;
volatile uint32_t ntp_req_flag = 0;
volatile uint32_t ntp_period = 60;
struct tm last_ntp_time;

static uint8_t push_server[4] = {192, 168, 0, 29};
static uint8_t push_msg_count = 0;

/* Extern variables ---------------------------------------------------------*/
extern I2C_ConfigStruct i2c;
extern const uint8_t spo2_table[];
extern SCHEDULE_LIST_TYPE schedule_list[];
extern uint8_t schedule_num;


/* Private functions ---------------------------------------------------------*/
void delay(__IO uint32_t milliseconds)
{
    TimingDelay = milliseconds;

    while (TimingDelay != 0) ;
}

void TimingDelay_Decrement(void)
{
    uwTick++;
    if (TimingDelay != 0x00) {
        TimingDelay--;
    }
}

uint32_t millis(void)
{
    return uwTick;
}

void UART_Config(void)
{
    UART_InitTypeDef UART_InitStructure;

    UART_StructInit(&UART_InitStructure);
    S_UART_Init(115200);
    S_UART_Cmd(ENABLE);
}

void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Direction = GPIO_Direction_OUT;
    GPIO_InitStructure.GPIO_AF = PAD_AF1;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void I2C_Config(void)
{
    i2c.scl_port = PORT_PC;
    i2c.scl_pin = GPIO_Pin_14;
    i2c.sda_port = PORT_PC;
    i2c.sda_pin = GPIO_Pin_13;

    I2C_Init(&i2c);

    I2C_Stop(&i2c);
    delay(100);
}

void DUALTIMER_Config(void)
{
    DUALTIMER_InitTypDef DUALTIMER_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    DUALTIMER_InitStructure.Timer_Load = GetSystemClock() / 1; //1s
    DUALTIMER_InitStructure.Timer_Prescaler = DUALTIMER_Prescaler_1;
    DUALTIMER_InitStructure.Timer_Wrapping = DUALTIMER_Periodic;
    DUALTIMER_InitStructure.Timer_Repetition = DUALTIMER_Wrapping;
    DUALTIMER_InitStructure.Timer_Size = DUALTIMER_Size_32;
    DUALTIMER_Init(DUALTIMER0_0, &DUALTIMER_InitStructure);

    DUALTIMER_ITConfig(DUALTIMER0_0, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DUALTIMER0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DUALTIMER_Cmd(DUALTIMER0_0, ENABLE);
}

void Network_Config(void)
{
    uint8_t mac_addr[6] = { 0x00, 0x08, 0xDC, 0x01, 0x02, 0x03 };

    memcpy(gWIZNETINFO.mac, mac_addr, 6);
    gWIZNETINFO.dhcp = NETINFO_DHCP;

    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);

    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], gWIZNETINFO.mac[2], gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
    printf("IP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
    printf("GW: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
    printf("SN: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
    printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
}

void dhcp_assign(void)
{
    getIPfromDHCP(gWIZNETINFO.ip);
    getGWfromDHCP(gWIZNETINFO.gw);
    getSNfromDHCP(gWIZNETINFO.sn);
    getDNSfromDHCP(gWIZNETINFO.dns);

    ctlnetwork(CN_SET_NETINFO, (void*) &gWIZNETINFO);
}

void dhcp_update(void)
{
    ;
}

void dhcp_conflict(void)
{
    ;
}

int get_int(const char *p, int str_len)
{
    char str_num[11];
    if( str_len > 10 || str_len <= 0 ) return 0;
    memcpy(str_num, p, str_len);
    str_num[str_len] = '\0';
    return atoi( str_num );
}

time_t parse_str_datetime(char *p)
{
    struct tm t;
    int len = strlen( p );
    if( len < 14 ) return 0;

    memset( &t, 0, sizeof(t) );

    t.tm_year = get_int( p, 4 ) - 1900;
    t.tm_mon  = get_int( p + 4, 2 ) - 1;
    t.tm_mday = get_int( p + 6, 2 );
    t.tm_hour = get_int( p + 8, 2 );
    t.tm_min  = get_int( p + 10, 2 );
    t.tm_sec  = get_int( p + 12, 2 );

    return mktime( &t );
}

int32_t send_push_message(uint8_t sn, uint8_t* rcv_buf, char *p_title, char *p_msg)
{
    int32_t ret;
    uint16_t size = 0;
    uint16_t any_port = 50000;

    close(sn);
    if ((ret = socket(sn, Sn_MR_TCP, any_port++, 0x00)) != sn) return ret;

    if ((ret = connect(sn, push_server, 80)) != SOCK_OK) return ret;

    if (getSn_SR(sn) == SOCK_ESTABLISHED) {
        if (getSn_IR(sn) & Sn_IR_CON) {
            setSn_IR(sn, Sn_IR_CON);

            int len = snprintf(push_body_buf, PUSH_BUF_SIZE,
                    "{\"message\":\"%s\",\"priority\":%d,\"title\":\"%s\"}",
                    p_msg, 5, p_title);

            snprintf(push_buf, PUSH_BUF_SIZE, "POST /message HTTP/1.1\r\n"
                        "X-Gotify-Key: A4nRR2evsApjXDs\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length: %d\r\n"
                        "Host: %d.%d.%d.%d\r\n\r\n"
                        "%.*s",
                        len,
                        push_server[0],
                        push_server[1],
                        push_server[2],
                        push_server[3],
                        len, push_body_buf);

            ret = send(sn, (uint8_t*)push_buf, strlen((const char *)push_buf));            
            if (ret < 0) {
                close(sn);
                return ret;
            }
        }

        if ((size = getSn_RX_RSR(sn)) > 0) {
            if (size > PUSH_BUF_SIZE) size = PUSH_BUF_SIZE;
            ret = recv(sn, rcv_buf, size);
            if (ret <= 0) return ret;
            printf("%s\r\n", rcv_buf);
        }
    }

    return -1;
}

void check_schedule_notify(const struct tm *new_ntp_time)
{
    if(schedule_num > 0){
        for(int i=0 ; i<MAX_SCHEDULE_NUM ; i++) {
            if(schedule_list[0].func &&
                (new_ntp_time->tm_mon == schedule_list[i].stime.tm_mon) &&
                (new_ntp_time->tm_mday == schedule_list[i].stime.tm_mday) &&
                (new_ntp_time->tm_hour == schedule_list[i].stime.tm_hour) &&
                (new_ntp_time->tm_min == schedule_list[i].stime.tm_min)) {
                schedule_list[i].push_req = 1;
                push_msg_count++;

                if(schedule_list[i].repeat == 0){
                    schedule_list[i].func = 0;
                    if(schedule_num > 0) schedule_num--;
                }
            }
        }
    }
}

void send_schedule_notify()
{
    if(schedule_num > 0 && push_msg_count){
        for(int i=0 ; i<MAX_SCHEDULE_NUM ; i++) {
            if(schedule_list[i].push_req) {
                send_push_message(PUSH_SOCKET_NUM, push_rcv_buf,
                    "Medication Alarm", schedule_list[i].content);
                schedule_list[i].push_req = 0;
            }
        }
    }
}

void get_ntp_datetime()
{
    struct tm t_new;
    time_t new_unix_time;

    while (SNTP_run(&new_unix_time) != 1);
    localtime_r(&new_unix_time, &t_new);
    ntp_period = SNTPCLK - t_new.tm_sec;
    check_schedule_notify(&t_new);
    memcpy(&last_ntp_time, &t_new, sizeof(struct tm));
}

void check_ntp_datetime()
{
    if(++ntp_timer_count == ntp_period) {
        ntp_req_flag = 1;
        ntp_timer_count = 0;
    }
}

int main(void)
{
    uint32_t ret;
    uint8_t dhcp_retry = 0;
    sense_type* p_sense = NULL;
    pulse_type* p_pulse_ir = NULL;
    pulse_type* p_pulse_red = NULL;
    ma_filter_type* p_ma_filter = NULL;
    uint32_t displaytime = 0;
    uint32_t ir_value = 0, red_value = 0;
    uint32_t now = 0, lastBeat = 0;

    SystemInit();

    SysTick_Config((GetSystemClock() / 1000));
    setTIC100US((GetSystemClock() / 10000));

    UART_Config();
    GPIO_Config();
    I2C_Config();
    DUALTIMER_Config();

    printf("W7500x Standard Peripheral Library version : %d.%d.%d\r\n",
        __W7500X_STDPERIPH_VERSION_MAIN, __W7500X_STDPERIPH_VERSION_SUB1, __W7500X_STDPERIPH_VERSION_SUB2);

    printf("SourceClock : %d\r\n", (int) GetSourceClock());
    printf("SystemClock : %d\r\n", (int) GetSystemClock());

    oled_init();
    oled_fill(0x0);
    oled_draw(1);

    printf("PHY Init : %s\r\n", PHY_Init(GPIOB, GPIO_Pin_15, GPIO_Pin_14) == SET ? "Success" : "Fail");
    printf("Link : %s\r\n", PHY_GetLinkStatus() == PHY_LINK_ON ? "On" : "Off");
    Network_Config();

    DHCP_init(0, RX_BUF);
    reg_dhcp_cbfunc(dhcp_assign, dhcp_assign, dhcp_conflict);
    if (gWIZNETINFO.dhcp == NETINFO_DHCP) {
        printf("Start DHCP\r\n");
        while (1) {
            ret = DHCP_run();

            if (ret == DHCP_IP_LEASED) {
                printf("DHCP Success\r\n");
                break;
            } else if (ret == DHCP_FAILED) {
                dhcp_retry++;
            }

            if (dhcp_retry > 3) {
                printf("DHCP Fail\r\n");
                break;
            }
        }
    }
    Network_Config();

    time_t ntp_datetime;
    struct tm t;
    time_t new_unix_time;

    memset(ntp_buf, 0x0, NTP_BUF_SIZE);
    SNTP_init(SNTP_SOCKET_NUM, ntp_server, 40, ntp_buf);
    while (SNTP_run(&ntp_datetime) != 1);

    localtime_r(&ntp_datetime, &t);
    ntp_period = SNTPCLK - t.tm_sec;
    memcpy(&last_ntp_time, &t, sizeof(struct tm));

	httpServer_init(TX_BUF, RX_BUF, MAX_HTTPSOCK, socknumlist);
	reg_httpServer_cbfunc(NVIC_SystemReset, NULL);
    reg_httpServer_webContent((uint8_t *)"index.html", (uint8_t *)index_page);

    printf("System Loop Start\r\n");
    oled_draw(2);

    p_sense = max30102_init();
    p_pulse_ir = pulse_init();
    p_pulse_red = pulse_init();
    p_ma_filter = ma_filter_init();
    p_wave = waveform_init();

    max30102_begin();
    max30102_setup();

    while (1) {
        if(ntp_req_flag) {
            get_ntp_datetime();
            ntp_req_flag = 0;
        }

        if(push_msg_count) {
            send_schedule_notify();
            push_msg_count = 0;
            I2C_Stop(&i2c);
        }

        httpServer_run(0);

        max30102_check(p_sense);

        now = millis();
        if (!max30102_available(p_sense)) {
            continue;
        }

        ir_value = max30102_getIR(p_sense);
        red_value = max30102_getRed(p_sense);

        max30102_nextSample(p_sense);

        if (ir_value < 15000) {
            beatAvg = 0;
            SPO2 = 0;
            SPO2f = 0;
            waveform_reset(p_wave);
            oled_draw(2);
        } else {
            int16_t ir_signal, red_signal;
            uint8_t beatRed, beatIR;

            ir_signal = pulse_dc_filter(p_pulse_ir, ir_value);
            red_signal = pulse_dc_filter(p_pulse_red, red_value);
            beatRed = pulse_isBeat(p_pulse_red, pulse_ma_filter(p_pulse_red, red_signal));
            beatIR = pulse_isBeat(p_pulse_ir, pulse_ma_filter(p_pulse_ir, ir_signal));

            waveform_record(p_wave, -ir_signal);

            if (beatIR) {
                long btpm = 60000 / (now - lastBeat);
                if (btpm > 0 && btpm < 200) {
                    beatAvg = bpm_filter(p_ma_filter, (int16_t)btpm);
                }
                lastBeat = now;

                long numerator = (pulse_avg_ac(p_pulse_red) * pulse_avg_dc(p_pulse_ir)) / 256;
                long denominator = (pulse_avg_dc(p_pulse_red) * pulse_avg_ac(p_pulse_ir)) / 256;
                int RX100 = (denominator > 0) ? (numerator * 100) / denominator : 999;

                SPO2f = (10400 - RX100 * 17 + 50) / 100;

                if ((RX100 >= 0) && (RX100 < 184))
                    SPO2 = spo2_table[RX100];
            }

            if (millis() - displaytime > 100) {
                displaytime = millis();
                waveform_scale(p_wave);
                oled_draw(3);
            }
        }

        GPIO_ToggleBits(GPIOC, GPIO_Pin_15);
    }

	return 0;
}

/******************** (C) COPYRIGHT WIZnet *****END OF FILE********************/
