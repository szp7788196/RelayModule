#ifndef __AT_PROTOCOL_H
#define __AT_PROTOCOL_H

#include "sys.h"
#include "common.h"

#define AT_MAX_NUM	26

#define RST				0
#define GMR				1
#define DEVNAME			2
#define DEVID			3
#define UUID			4
#define RELAY			5
#define AREA			6
#define BOX				7


typedef struct AT_Command
{
	char *cmd;
	u8 len;
	u8 id;
}AT_Command_S;



extern AT_Command_S AT_CommandBuf[AT_MAX_NUM];

/******************************************************************************************
/                                    通讯错误码
/	
/	
/	
/	
/	
/	
/	
/
******************************************************************************************/
void AT_CommandInit(void);
u16 AT_CommandDataAnalysis(u8 *buf,u16 len,u8 *outbuf,u8 *hold_reg);
u8 PackAT_CommandRespone(u8 *inbuf,u8 err_code,u8 *respbuf,u8 *outbuf);


u8 AT_CommandRST(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandGMR(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandDEVNAME(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandDEVID(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandUUID(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandRELAY(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandDeviceAreaID(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);
u8 AT_CommandDeviceBoxID(u8 cmd_id,u8 *inbuf,u16 inbuf_len,u8 *respbuf);









































#endif
