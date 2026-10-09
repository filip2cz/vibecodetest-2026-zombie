#include "raylib.h"
int main() {
    InitWindow(800, 450, "test");
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, -10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    SetTargetFPS(60);
    int frames = 0;
    while (!WindowShouldClose() && frames < 10) {
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawCube((Vector3){0,0,0}, 2, 2, 2, RED);
        EndMode3D();
        EndDrawing();
        frames++;
    }
    CloseWindow();
    return 0;
}
