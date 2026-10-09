#include "raylib.h"
#include <stdio.h>

int main() {
    InitWindow(800, 450, "test");
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    // Simulate one frame update
    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    printf("Camera pos: %f %f %f\n", camera.position.x, camera.position.y, camera.position.z);
    printf("Camera target: %f %f %f\n", camera.target.x, camera.target.y, camera.target.z);
    
    CloseWindow();
    return 0;
}
