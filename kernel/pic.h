#pragma once

void pic_remap(uint8_t master_offset, uint8_t slave_offset);
void pic_mask_all_interrupts(void);
void pic_send_end_of_interrupt();
