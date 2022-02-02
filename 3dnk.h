#ifndef TDNK_H
#define TDNK_H

#include <3dmr/render/viewer.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT

#include "nuklear.h"

struct NKRenderer {
    struct Viewer* viewer;

    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
};

int tdnk_init(struct NKRenderer* renderer, struct Viewer* viewer);
void tdnk_shutdown(struct NKRenderer *dev);

void tdnk_render(struct NKRenderer *dev,
                 struct nk_context *ctx,
                 struct nk_vec2 scale,
                 enum nk_anti_aliasing AA);

void tdnk_upload_atlas(struct NKRenderer *dev,
                       const void *image,
                       int width, int height);
#endif
