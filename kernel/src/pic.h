#pragma once

void pic_remap(u8 master_offset, u8 slave_offset);
void pic_mask_all_interrupts(void);
void pic_send_end_of_interrupt();
