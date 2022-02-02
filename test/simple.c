#include <stdlib.h>
#include <stdio.h>

#include <3dmr/scene/scene.h>
#include <3dmr/render/lights_buffer_object.h>
#include <3dmr/render/camera_buffer_object.h>

#include <3dasset.h>
#include <3dnk.h>

struct UI {
    struct NKRenderer rndr;
    struct nk_font_atlas atlas;
    struct nk_font* font;
    struct nk_context ctx;
};

struct Prog {
    struct Scene scene;
    struct Camera camera;
    struct Lights lights;

    struct UI ui;
};

static float scale = 1.;

static void resize_callback(struct Viewer* viewer, void* data) {
    struct Prog* prog = data;
    struct Camera* cam = &prog->camera;
    glViewport(0, 0, viewer->width, viewer->height);
    camera_set_ratio(((float)viewer->width) / ((float)viewer->height), cam->projection);
    camera_buffer_object_update_projection(&prog->scene.camera, MAT_CONST_CAST(cam->projection));
    uniform_buffer_send(&prog->scene.camera);
}

static int setup_lights(struct Scene* scene, struct Lights* lights) {
    struct DirectionalLight* dl;

    light_init(lights);
    dl = &lights->directional[0];

    set3v(dl->color, 1, 1, 1);
    set3v(dl->direction, -0.9, -0.3, -1);
    lights->numDirectionalLights++;

    lights_buffer_object_update(&scene->lights, lights);
    uniform_buffer_send(&scene->lights);
    return 1;
}

static int setup_camera(struct Scene* scene, struct Camera* cam) {
    Vec3 pos = {5, 5, 5}, lookAt = {0}, up = {0, 0, 1};

    camera_projection(1, 2 * M_PI * 60. / 360., 0.1, 100, cam->projection);
    camera_look_at(pos, lookAt, up, cam->view);

    camera_buffer_object_update_projection(&scene->camera, MAT_CONST_CAST(cam->projection));
    camera_buffer_object_update_view_and_position(&scene->camera, MAT_CONST_CAST(cam->view));
    uniform_buffer_send(&scene->camera);
    return 1;
}

static int ui_init(struct UI* ui, struct Viewer* v) {
    struct nk_font_config cfg;
    const void* image;
    int w, h;

    if (!tdnk_init(&ui->rndr, v)) return 0;

    cfg = nk_font_config(0);
    nk_font_atlas_init_default(&ui->atlas);
    nk_font_atlas_begin(&ui->atlas);
    if (!(ui->font = nk_font_atlas_add_from_file(&ui->atlas,
                                                 "font.ttf",
                                                 20.0f,
                                                 &cfg))) {
        fprintf(stderr, "Error: can't load font\n");
        return 0;
    } else if (!(image = nk_font_atlas_bake(&ui->atlas, &w, &h,
                                            NK_FONT_ATLAS_RGBA32))) {
        fprintf(stderr, "Error: can't bake font\n");
        return 0;
    }

    tdnk_upload_atlas(&ui->rndr, image, w, h);
    nk_font_atlas_end(&ui->atlas,
                      nk_handle_id((int)ui->rndr.font_tex),
                      &ui->rndr.null);
    nk_init_default(&ui->ctx, &ui->font->handle);

    return 1;
}

void ui_free(struct UI* ui) {
    nk_font_atlas_clear(&ui->atlas);
    nk_free(&ui->ctx);
    tdnk_shutdown(&ui->rndr);
}

void ui_update(struct UI* ui, struct Viewer* viewer) {
    static unsigned long prog = 50;

    nk_input_begin(&ui->ctx);
    nk_input_motion(&ui->ctx, viewer->cursorPos[0], viewer->cursorPos[1]);
    nk_input_button(&ui->ctx,
                    NK_BUTTON_LEFT,
                    viewer->cursorPos[0],
                    viewer->cursorPos[1],
                    viewer->buttonPressed[0]);
    nk_input_button(&ui->ctx,
                    NK_BUTTON_MIDDLE,
                    viewer->cursorPos[0],
                    viewer->cursorPos[1],
                    viewer->buttonPressed[1]);
    nk_input_button(&ui->ctx,
                    NK_BUTTON_RIGHT,
                    viewer->cursorPos[0],
                    viewer->cursorPos[1],
                    viewer->buttonPressed[2]);
    nk_input_end(&ui->ctx);

    nk_begin(&ui->ctx, "test", nk_rect(0, 100, 275, 610),
            NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE);
    nk_layout_row_dynamic(&ui->ctx, 30, 2);
    nk_label(&ui->ctx, "Scale:", NK_TEXT_RIGHT);
    nk_progress(&ui->ctx, &prog, 100, nk_true);
    scale = (float) prog / 50.;
    nk_end(&ui->ctx);
}

int main(int argc, char** argv) {
    struct Prog prog;
    struct Node* cube;
    struct MaterialConfig mat;
    struct Viewer* viewer;

    viewer = viewer_new(1024, 1024, argv[0]);

    scene_init(&prog.scene, NULL);
    setup_lights(&prog.scene, &prog.lights);
    setup_camera(&prog.scene, &prog.camera);

    asset_mat_pbr_color(&mat, 0.7, 0.1, 0, 0, 0.3);
    cube = asset_box(&mat, 1, 1, 1);

    node_add_child(&prog.scene.root, cube);

    viewer->resize_callback = resize_callback;
    viewer->callbackData = &prog;

    if (!ui_init(&prog.ui, viewer)) return 1;

    glClearColor(0.3, 0.3, 0.3, 1);
    while (1) {
        Vec3 s;

        viewer_next_frame(viewer);
        viewer_process_events(viewer);
        ui_update(&prog.ui, viewer);

        set3v(s, scale, scale, scale);
        node_set_scale(cube, s);
        scene_update_nodes(&prog.scene, NULL, NULL);
        scene_update_render_queue(&prog.scene,
                                  MAT_CONST_CAST(prog.camera.view),
                                  MAT_CONST_CAST(prog.camera.projection));

        scene_render(&prog.scene, NULL);
        tdnk_render(&prog.ui.rndr,
                    &prog.ui.ctx,
                    nk_vec2(2, 2),
                    NK_ANTI_ALIASING_ON);
    }
    ui_free(&prog.ui);
    return 0;
}
