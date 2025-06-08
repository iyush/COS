#pragma once

void pic_remap(s8 master_offset, s8 slave_offset);
void pic_disable_all_interrupts(void);
void pic_send_end_of_interrupt();
