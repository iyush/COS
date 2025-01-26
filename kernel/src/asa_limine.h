#ifndef __ASA_LIMINE
#define __ASA_LIMINE

// typedeffing because limine.h declares its own uint semantics
typedef u64 uint64_t;
typedef u32 uint32_t;
typedef u16 uint16_t;
typedef u8 uint8_t;


#include <limine.h>
#include "assert.h"

// Set the base revision to 2, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.
__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request       = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};
__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request                 = { .id = LIMINE_MEMMAP_REQUEST };
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address_request = { .id = LIMINE_KERNEL_ADDRESS_REQUEST };
__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request                     = { .id = LIMINE_HHDM_REQUEST };
__attribute__((used, section(".requests"))) static volatile struct limine_kernel_file_request kfile_request             = { .id = LIMINE_KERNEL_FILE_REQUEST };
__attribute__((used, section(".requests"))) static volatile struct limine_module_request module_request                 = { .id = LIMINE_MODULE_REQUEST };


// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;


// Frame describes a physical address.
typedef struct Frame {
    u64 ptr;
} Frame;

Frame frame_create(u64 ptr) {
    return (Frame){
        .ptr = ptr
    };
}

typedef struct Context {
    u64 is_set;
    u64 hhdm_offset;
} Context;

static Context ctx;

void context_init() {
    ASSERT(LIMINE_BASE_REVISION_SUPPORTED)
    ASSERT(hhdm_request.response);
    ASSERT(framebuffer_request.response);
    ASSERT(framebuffer_request.response->framebuffer_count > 0);
    ASSERT(memmap_request.response);
    ASSERT(kernel_address_request.response);
    ASSERT(kfile_request.response);
    ASSERT(module_request.response);

    ctx = (Context){
        .is_set = 1,
        .hhdm_offset = hhdm_request.response->offset
    };
    return;
}


Frame to_lower_half(u64 address) {
	ASSERT(ctx.is_set);
	return frame_create(address - ctx.hhdm_offset);
}

// only frames are supposed to be in lower half, so we enforce that here.
u64 to_higher_half(Frame frame_ptr) {
	ASSERT(ctx.is_set);
	return frame_ptr.ptr + ctx.hhdm_offset;
}

#endif
