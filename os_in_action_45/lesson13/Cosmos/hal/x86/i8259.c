/**********************************************************
		i8259中断控制器源文件i8259.c
***********************************************************
				彭东
**********************************************************/
#include "cosmostypes.h"
#include "cosmosmctrl.h"
// 多个设备的中断信号线都会连接到中断控制器上，中断控制器可以决定启用或者屏蔽哪些设备的中断，还可以决定设备中断之间的优先线，所以它才叫中断控制器
// x86 平台使用了两片 8259A 芯片，以级联的方式存在, 它拥有 15 个中断源（即可以有 15 个中断信号接入）
void init_i8259()
{
	//初始化主从8259a
	out_u8_p(ZIOPT, ICW1);
	out_u8_p(SIOPT, ICW1);
	out_u8_p(ZIOPT1, ZICW2);
	out_u8_p(SIOPT1, SICW2);
	out_u8_p(ZIOPT1, ZICW3);
	out_u8_p(SIOPT1, SICW3);
	out_u8_p(ZIOPT1, ICW4);
	out_u8_p(SIOPT1, ICW4);
	//屏蔽全部中断源, 因为在初始化阶段还不能处理中断
	out_u8_p(ZIOPT1, 0xff);
	out_u8_p(SIOPT1, 0xff);
	
	return;
}

void i8259_send_eoi()
{
	out_u8_p(_INTM_CTL, _EOI);
	out_u8_p(_INTS_CTL, _EOI);
	return;
}

void i8259_enabled_line(u32_t line)
{
	cpuflg_t flags;
	save_flags_cli(&flags);
	if (line < 8)
	{
		u8_t amtmp = in_u8(_INTM_CTLMASK);
		amtmp &= (u8_t)(~(1 << line));
		out_u8_p(_INTM_CTLMASK, amtmp);
	}
	else
	{
		u8_t astmp = in_u8(_INTM_CTLMASK);
		astmp &= (u8_t)(~(1 << 2));
		out_u8_p(_INTM_CTLMASK, astmp);
		astmp = in_u8(_INTS_CTLMASK);
		astmp &= (u8_t)(~(1 << (line - 8)));
		out_u8_p(_INTS_CTLMASK, astmp);
	}
	restore_flags_sti(&flags);
	return;
}

void i8259_disable_line(u32_t line)
{
	cpuflg_t flags;
	save_flags_cli(&flags);
	if (line < 8)
	{
		u8_t amtmp = in_u8(_INTM_CTLMASK);
		amtmp |= (u8_t)((1 << line));
		out_u8_p(_INTM_CTLMASK, amtmp);
	}
	else
	{
		u8_t astmp = in_u8(_INTM_CTLMASK);
		astmp |= (u8_t)((1 << 2));
		out_u8_p(_INTM_CTLMASK, astmp);
		astmp = in_u8(_INTS_CTLMASK);
		astmp |= (u8_t)((1 << (line - 8)));
		out_u8_p(_INTS_CTLMASK, astmp);
	}
	restore_flags_sti(&flags);
	return;
}

void i8259_save_disableline(u64_t *svline, u32_t line)
{
	u32_t intftmp;
	cpuflg_t flags;
	save_flags_cli(&flags);
	u8_t altmp = in_u8(_INTM_CTLMASK);
	intftmp = altmp;
	altmp = in_u8(_INTS_CTLMASK);
	intftmp = (intftmp << 8) | altmp;
	*svline = intftmp;
	i8259_disable_line(line);

	restore_flags_sti(&flags);
	return;
}

void i8259_rest_enabledline(u64_t *svline, u32_t line)
{
	cpuflg_t flags;
	save_flags_cli(&flags);

	u32_t intftmp = (u32_t)(*svline);

	u8_t altmp = (intftmp & 0xff);
	out_u8_p(_INTS_CTLMASK, altmp);
	altmp = (u8_t)(intftmp >> 8);
	out_u8_p(_INTM_CTLMASK, altmp);

	restore_flags_sti(&flags);

	return;
}
