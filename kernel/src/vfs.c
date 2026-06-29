#include <stddef.h>
#include <stdint.h>

// VFS node types
typedef enum {
    VFS_NODE_FILE,
    VFS_NODE_DIR,
    VFS_NODE_DEV
} vfs_node_type_t;

// Forward declaration
struct vfs_node;

// File operations structure
typedef struct {
    int (*open)(struct vfs_node *node);
    int (*close)(struct vfs_node *node);
    ssize_t (*read)(struct vfs_node *node, void *buf, size_t size, off_t offset);
    ssize_t (*write)(struct vfs_node *node, const void *buf, size_t size, off_t offset);
} vfs_ops_t;

// VFS node structure
typedef struct vfs_node {
    char *name;
    vfs_node_type_t type;
    struct vfs_node *parent;
    struct vfs_node **children;
    size_t child_count;
    vfs_ops_t *ops;
    void *data; // Filesystem-specific data
} vfs_node_t;

// Root of the VFS
vfs_node_t *vfs_root = NULL;

// VFS operation prototypes
vfs_node_t *vfs_create_node(const char *name, vfs_node_type_t type, vfs_node_t *parent, vfs_ops_t *ops);
int vfs_mount(vfs_node_t *mountpoint, vfs_node_t *node);
vfs_node_t *vfs_lookup(vfs_node_t *dir, const char *name);
ssize_t vfs_read(vfs_node_t *node, void *buf, size_t size, off_t offset);
ssize_t vfs_write(vfs_node_t *node, const void *buf, size_t size, off_t offset);

// Basic implementations

vfs_node_t *vfs_create_node(const char *name, vfs_node_type_t type, vfs_node_t *parent, vfs_ops_t *ops) {
    // Allocate and initialize a new VFS node
    // (Implementation omitted for brevity)
    return NULL;
}

int vfs_mount(vfs_node_t *mountpoint, vfs_node_t *node) {
    // Mount a filesystem node at the given mountpoint
    // (Implementation omitted for brevity)
    return 0;
}

vfs_node_t *vfs_lookup(vfs_node_t *dir, const char *name) {
    // Search for a child node by name in the given directory
    // (Implementation omitted for brevity)
    return NULL;
}

ssize_t vfs_read(vfs_node_t *node, void *buf, size_t size, off_t offset) {
    if (node && node->ops && node->ops->read) {
        return node->ops->read(node, buf, size, offset);
    }
    return -1;
}

ssize_t vfs_write(vfs_node_t *node, const void *buf, size_t size, off_t offset) {
    if (node && node->ops && node->ops->write) {
        return node->ops->write(node, buf, size, offset);
    }
    return -1;
}