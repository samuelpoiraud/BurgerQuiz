/*
 * burger_dialog.c
 *
 *  Created on: 8 oct. 2018
 *      Author: Nirgal
 */

#include "stm32f1_uart.h"
#include "config.h"
#include "burger_dialog.h"
#include "led.h"


#define UART_ID	UART2_ID	//Configuration du p�riph�rique UART choisi

/*
Norme des messages transmis :

SOH 	MSGID	SIZE	DATA(s)	EOT
01		HH		01		HH		04

MSGID :
Ping	16
Pong 	06
LED		ED
Button	B0

Ping :
01 16 00 04

Pong :
01 06 00 04

LED ON :
01 ED 01 01 04

LED OFF :
01 ED 01 00 04

Button pressed :
01 B0 00 04
*/




static void display_msg(msg_id_e id, uint8_t size, uint8_t * datas);
static void BURGER_DIALOG_process_msg(msg_id_e id, uint8_t size, uint8_t * datas);

/**
 * @brief	Cette fonction assure le traitement des caract�res re�us sur l'UART_ID indiqu�e. Les ocets sont rang�s dans les variables msg_id, size et datas, avant d'�tre trait� lorsque le message complet et valide est re�u.
 * @post	La fonction BURGER_DIALOG_process_msg() sera appel� si un message au format valide est re�u
 * @pre		Cette fonction doit �tre appel�e en boucle dans la t�che de fond (au moins une fois pour chaque caract�re re�u)
 */
void BURGER_DIALOG_process_main(void)
{
	uint8_t c;
	static uint8_t datas[255];
	static uint16_t index = 0;
	static msg_id_e msg_id;
	static uint8_t size;

	if(UART_data_ready(UART_ID))
	{
		c = UART_getc(UART_ID);
		switch(index)
		{
			case 0:
				if(c == SOH)
					index++;
				break;
			case 1:
				msg_id = (msg_id_e)c;
				index++;
				break;
			case 2:
				size = c;
				index++;
				break;
			default:
				if(index-3 == size)
				{
					if(c == EOT)//ok, fin du message !
					{
						BURGER_DIALOG_process_msg(msg_id, size, datas);
					}
					index = 0;
				}
				else if(index-3 < size)
				{
					datas[index-3] = c;
					index++;
				}
				else if(c == 0x01)
					index = 1;
				else
					index = 0;
				break;
		}
	}
}


/**
 * @brief	Cette fonction permet l'envoi d'un message sur la liaison s�rie.
 * @pre		Le tableau datas doit contenir au moins 'size' octet. Sinon, le pointeur 'datas' peut �tre NULL.
 */
void BURGER_DIALOG_send_msg(msg_id_e id, uint8_t size, uint8_t * datas)
{
	uint8_t i;
	if(size > 0 && datas == NULL)
		return;
	UART_putc(UART_ID, SOH);			//SOH
	UART_putc(UART_ID, (uint8_t)id);	//MSG ID
	UART_putc(UART_ID, size);			//size
	for(i=0; i<size; i++)
		UART_putc(UART_ID, datas[i]);	//datas...
	UART_putc(UART_ID, EOT);			//EOT
}

/**
 * @brief	Cette fonction traite le message re�u et agit en cons�quence.
 */
static void BURGER_DIALOG_process_msg(msg_id_e id, uint8_t size, uint8_t * datas)
{
	display_msg(id, size, datas);
	switch(id)
	{
		case MSG_PING:
			if(size == 0)
				BURGER_DIALOG_send_msg(MSG_PONG, 0, NULL);
			break;
		case MSG_PONG:
			//nothing
			break;
		case MSG_LED:
			if(size == 1)
			{
				if(datas[0] <= LED_STATE_NB)
					LED_set(datas[0]);

			}
			break;
		case MSG_BUTTON:
			//nothing
			break;
		default:

			break;
	}
}

/**
 * @brief Cette fonction priv�e affiche le contenu d'un message donn� en param�tre
 */
static void display_msg(msg_id_e id, uint8_t size, uint8_t * datas)
{
	switch(id)
	{
		case MSG_PING:
			printf("Ping");
			break;
		case MSG_PONG:
			printf("Pong");
			break;
		case MSG_LED:
			printf("LED");
			break;
		case MSG_BUTTON:
			printf("Button");
			break;
		default:
			printf("Unknow msg id");
			break;
	}
	printf(" [%x data%s ", size, (size>1)?"s]":"]");
	for(uint8_t i = 0 ; i<size ; i++)
		printf("%02x ", datas[i]);
	printf("\n");
}

