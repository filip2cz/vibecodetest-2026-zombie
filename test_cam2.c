#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

int main() {
    InitWindow(800, 450, "test");
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    DisableCursor();
    
    float cameraYaw = 0.0f;
    float cameraPitch = 0.0f;
    
    SetTargetFPS(60);
    int frames = 0;
    while (!WindowShouldClose() && frames < 60) {
        float dt = GetFrameTime();
        
        Vector2 mouseDelta = GetMouseDelta();
        
        // Ignore massive deltas on first few frames
        if (frames < 5 && (mouseDelta.x > 100 || mouseDelta.y > 100 || mouseDelta.x < -100 || mouseDelta.y < -100)) {
            mouseDelta.x = 0;
            mouseDelta.y = 0;
        }

        cameraYaw -= mouseDelta.x * 0.003f;
        cameraPitch -= mouseDelta.y * 0.003f;
        
        if (cameraPitch > 1.5f) cameraPitch = 1.5f;
        if (cameraPitch < -1.5f) cameraPitch = -1.5f;
        
        Vector3 forward = {
            cosf(cameraPitch) * sinf(cameraYaw),
            sinf(cameraPitch),
            cosf(cameraPitch) * cosf(cameraYaw)
        };
        
        Vector3 right = {
            cosf(cameraYaw),
            0.0f,
            -sinf(cameraYaw)
        };
        
        camera.position.y = 2.0f;
        camera.target = Vector3Add(camera.position, forward);
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawCube((Vector3){0,0,5}, 2, 2, 2, RED);
        EndMode3D();
        EndDrawing();
        frames++;
    }
    CloseWindow();
    return 0;
}
