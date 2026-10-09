#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "rlgl.h"

#define MAX_ZOMBIES 100
#define MAX_PARTICLES 500

typedef struct Zombie {
    Vector3 position;
    bool active;
    float speed;
    float size;
    float animPhase;
} Zombie;

typedef struct Particle {
    Vector3 position;
    Vector3 velocity;
    float life;
    float maxLife;
    bool active;
    Color color;
} Particle;

int main(void)
{
    const int screenWidth = 1024;
    const int screenHeight = 768;

    SetConfigFlags(FLAG_MSAA_4X_HINT); // Enable anti-aliasing
    InitWindow(screenWidth, screenHeight, "Simple 3D Zombie Shooter");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 1.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    DisableCursor();

    Zombie zombies[MAX_ZOMBIES] = { 0 };
    Particle particles[MAX_PARTICLES] = { 0 };
    
    int score = 0;
    int health = 100;
    float spawnTimer = 0.0f;
    bool gameOver = false;
    float flashAlpha = 0.0f; // Muzzle flash
    float recoilOffset = 0.0f; // Gun recoil

    SetTargetFPS(60);

    // Colors
    Color skyColor = (Color){ 15, 20, 30, 255 };
    Color groundColor = (Color){ 30, 35, 45, 255 };
    Color gridColor = (Color){ 50, 60, 70, 255 };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (!gameOver) {
            // Camera Logic
            static float cameraYaw = 0.0f;
            static float cameraPitch = 0.0f;
            
            Vector2 mouseDelta = GetMouseDelta();
            if (mouseDelta.x > 100 || mouseDelta.y > 100 || mouseDelta.x < -100 || mouseDelta.y < -100) {
                mouseDelta.x = 0;
                mouseDelta.y = 0;
            }

            cameraYaw -= mouseDelta.x * 0.002f;
            cameraPitch -= mouseDelta.y * 0.002f;
            
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
            
            float moveSpeed = 8.0f * dt;
            if (IsKeyDown(KEY_W)) camera.position = Vector3Add(camera.position, Vector3Scale(forward, moveSpeed));
            if (IsKeyDown(KEY_S)) camera.position = Vector3Subtract(camera.position, Vector3Scale(forward, moveSpeed));
            if (IsKeyDown(KEY_A)) camera.position = Vector3Add(camera.position, Vector3Scale(right, moveSpeed));
            if (IsKeyDown(KEY_D)) camera.position = Vector3Subtract(camera.position, Vector3Scale(right, moveSpeed));
            
            camera.position.y = 2.0f;
            camera.target = Vector3Add(camera.position, forward);

            // Spawning Zombies
            spawnTimer += dt;
            float spawnRate = (score > 100) ? 0.5f : 1.5f; // Faster spawn later
            if (spawnTimer > spawnRate) {
                spawnTimer = 0.0f;
                for (int i = 0; i < MAX_ZOMBIES; i++) {
                    if (!zombies[i].active) {
                        zombies[i].active = true;
                        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                        float distance = (float)GetRandomValue(30, 60);
                        zombies[i].position.x = camera.position.x + cosf(angle) * distance;
                        zombies[i].position.z = camera.position.z + sinf(angle) * distance;
                        zombies[i].position.y = 0.0f;
                        zombies[i].speed = (float)GetRandomValue(20, 60) / 10.0f;
                        zombies[i].size = 0.6f;
                        zombies[i].animPhase = (float)GetRandomValue(0, 10);
                        break;
                    }
                }
            }

            // Update Zombies
            for (int i = 0; i < MAX_ZOMBIES; i++) {
                if (zombies[i].active) {
                    Vector3 dir = Vector3Subtract(camera.position, zombies[i].position);
                    dir.y = 0;
                    if (Vector3Length(dir) > 0) dir = Vector3Normalize(dir);
                    
                    zombies[i].position.x += dir.x * zombies[i].speed * dt;
                    zombies[i].position.z += dir.z * zombies[i].speed * dt;
                    zombies[i].animPhase += dt * 10.0f;

                    if (Vector3Distance(camera.position, (Vector3){zombies[i].position.x, 2.0f, zombies[i].position.z}) < 2.0f) {
                        health -= 15;
                        zombies[i].active = false;
                        
                        // Hurt effect (red screen)
                        flashAlpha = 1.0f;
                        
                        if (health <= 0) gameOver = true;
                    }
                }
            }

            // Update Particles
            for (int i = 0; i < MAX_PARTICLES; i++) {
                if (particles[i].active) {
                    particles[i].position = Vector3Add(particles[i].position, Vector3Scale(particles[i].velocity, dt));
                    particles[i].velocity.y -= 9.8f * dt; // Gravity
                    particles[i].life -= dt;
                    if (particles[i].position.y < 0.1f) particles[i].position.y = 0.1f; // Hit ground
                    if (particles[i].life <= 0) particles[i].active = false;
                }
            }

            // Recoil recovery
            if (recoilOffset > 0.0f) {
                recoilOffset -= 200.0f * dt;
                if (recoilOffset < 0) recoilOffset = 0;
            }

            // Shooting
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                flashAlpha = 1.0f; // Muzzle flash
                recoilOffset = 30.0f; // Kickback

                Ray ray = GetMouseRay((Vector2){ screenWidth/2.0f, screenHeight/2.0f }, camera);
                int hitIndex = -1;
                float closestDist = 9999.0f;

                for (int i = 0; i < MAX_ZOMBIES; i++) {
                    if (zombies[i].active) {
                        BoundingBox box = {
                            (Vector3){ zombies[i].position.x - zombies[i].size, 0, zombies[i].position.z - zombies[i].size },
                            (Vector3){ zombies[i].position.x + zombies[i].size, 2.5f, zombies[i].position.z + zombies[i].size }
                        };
                        RayCollision hit = GetRayCollisionBox(ray, box);
                        if (hit.hit && hit.distance < closestDist) {
                            closestDist = hit.distance;
                            hitIndex = i;
                        }
                    }
                }

                if (hitIndex != -1) {
                    zombies[hitIndex].active = false;
                    score += 10;
                    
                    // Spawn blood particles
                    for (int p = 0; p < 20; p++) {
                        for (int k = 0; k < MAX_PARTICLES; k++) {
                            if (!particles[k].active) {
                                particles[k].active = true;
                                particles[k].position = (Vector3){ zombies[hitIndex].position.x, 1.5f, zombies[hitIndex].position.z };
                                particles[k].velocity = (Vector3){
                                    (GetRandomValue(-100, 100) / 10.0f),
                                    (GetRandomValue(50, 150) / 10.0f),
                                    (GetRandomValue(-100, 100) / 10.0f)
                                };
                                particles[k].life = GetRandomValue(5, 15) / 10.0f;
                                particles[k].maxLife = particles[k].life;
                                particles[k].color = (Color){ GetRandomValue(150, 220), 0, 0, 255 };
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            if (IsKeyPressed(KEY_ENTER)) {
                gameOver = false;
                health = 100;
                score = 0;
                for (int i = 0; i < MAX_ZOMBIES; i++) zombies[i].active = false;
                for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
                camera.position = (Vector3){ 0.0f, 2.0f, 0.0f };
            }
        }

        // --- DRAWING ---
        BeginDrawing();
            ClearBackground(skyColor);

            if (!gameOver) {
                BeginMode3D(camera);
                    // Environment
                    DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 400.0f, 400.0f }, groundColor);
                    
                    // Fade out grid at distance using alpha (illusion of fog)
                    rlPushMatrix();
                    rlTranslatef(0, 0.01f, 0); // Slightly above ground
                    DrawGrid(100, 4.0f); // Make grid squares larger
                    rlPopMatrix();

                    // Draw Zombies
                    for (int i = 0; i < MAX_ZOMBIES; i++) {
                        if (zombies[i].active) {
                            Vector3 p = zombies[i].position;
                            float s = zombies[i].size;
                            
                            // Body parts
                            Color skin = (Color){ 80, 140, 80, 255 };
                            Color clothes = (Color){ 60, 50, 80, 255 };
                            Color blood = MAROON;
                            
                            // Torso
                            DrawCube((Vector3){ p.x, p.y + 1.2f, p.z }, s, 1.0f, s, clothes);
                            DrawCubeWires((Vector3){ p.x, p.y + 1.2f, p.z }, s, 1.0f, s, BLACK);
                            
                            // Head
                            float bob = sinf(zombies[i].animPhase) * 0.1f;
                            Vector3 headPos = (Vector3){ p.x, p.y + 2.0f + bob, p.z };
                            DrawCube(headPos, s*0.8f, s*0.8f, s*0.8f, skin);
                            
                            // Eyes
                            Vector3 dir = Vector3Subtract(camera.position, p);
                            dir.y = 0;
                            if (Vector3Length(dir) > 0) dir = Vector3Normalize(dir);
                            Vector3 eyePos = headPos;
                            eyePos.x += dir.x * (s*0.4f);
                            eyePos.z += dir.z * (s*0.4f);
                            DrawCube(eyePos, s*0.6f, s*0.15f, s*0.6f, RED);
                            
                            // Arms (Zombie pose)
                            Vector3 leftArmPos = (Vector3){ p.x - dir.z * s*0.8f, p.y + 1.4f, p.z + dir.x * s*0.8f };
                            Vector3 rightArmPos = (Vector3){ p.x + dir.z * s*0.8f, p.y + 1.4f, p.z - dir.x * s*0.8f };
                            
                            // Reach arms forward
                            leftArmPos.x += dir.x * 0.6f; leftArmPos.z += dir.z * 0.6f;
                            rightArmPos.x += dir.x * 0.6f; rightArmPos.z += dir.z * 0.6f;
                            
                            DrawCube(leftArmPos, 0.3f, 0.3f, 0.3f, skin);
                            DrawCube(rightArmPos, 0.3f, 0.3f, 0.3f, skin);

                            // Legs
                            float legSwing = sinf(zombies[i].animPhase * 0.5f);
                            Vector3 leftLeg = (Vector3){ p.x - dir.z * 0.3f + dir.x * legSwing * 0.4f, p.y + 0.4f, p.z + dir.x * 0.3f + dir.z * legSwing * 0.4f };
                            Vector3 rightLeg = (Vector3){ p.x + dir.z * 0.3f - dir.x * legSwing * 0.4f, p.y + 0.4f, p.z - dir.x * 0.3f - dir.z * legSwing * 0.4f };
                            
                            DrawCube(leftLeg, 0.4f, 0.8f, 0.4f, clothes);
                            DrawCube(rightLeg, 0.4f, 0.8f, 0.4f, clothes);
                        }
                    }

                    // Draw Particles (Blood)
                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].active) {
                            float alpha = particles[i].life / particles[i].maxLife;
                            Color c = particles[i].color;
                            c.a = (unsigned char)(255 * alpha);
                            DrawCube(particles[i].position, 0.1f, 0.1f, 0.1f, c);
                        }
                    }

                EndMode3D();

                // HUD & UI (2D)
                
                // Gun in 2D Space (Doom style)
                int gunX = screenWidth / 2 + 150;
                int gunY = screenHeight - 200 + (int)recoilOffset;
                // Gun body
                DrawRectangle(gunX, gunY, 80, 250, DARKGRAY);
                DrawRectangle(gunX - 10, gunY + 40, 100, 60, GRAY);
                // Gun barrel
                DrawRectangle(gunX + 25, gunY - 100, 30, 100, BLACK);
                
                // Muzzle flash
                if (flashAlpha > 0.0f) {
                    DrawCircle(gunX + 40, gunY - 120, 50 * flashAlpha, ORANGE);
                    DrawCircle(gunX + 40, gunY - 120, 25 * flashAlpha, YELLOW);
                    flashAlpha -= dt * 10.0f; // Fade out rapidly
                }

                // Crosshair
                DrawLine(screenWidth/2 - 15, screenHeight/2, screenWidth/2 + 15, screenHeight/2, GREEN);
                DrawLine(screenWidth/2, screenHeight/2 - 15, screenWidth/2, screenHeight/2 + 15, GREEN);
                DrawCircleLines(screenWidth/2, screenHeight/2, 10, DARKGREEN);

                // Health bar
                DrawText("HEALTH", 30, 30, 20, WHITE);
                DrawRectangle(30, 60, 200, 30, DARKGRAY);
                DrawRectangle(30, 60, health * 2, 30, (health > 30) ? GREEN : RED);
                DrawRectangleLines(30, 60, 200, 30, WHITE);

                // Score
                DrawText(TextFormat("SCORE: %05d", score), screenWidth - 250, 30, 30, WHITE);
                
                // Vignette / Damage screen effect
                if (health < 100 && health > 0) {
                    float hurtIntensity = (100 - health) / 100.0f;
                    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(RED, hurtIntensity * 0.2f));
                }

            } else { // Game Over
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
                DrawText("YOU DIED", screenWidth/2 - MeasureText("YOU DIED", 80)/2, screenHeight/2 - 100, 80, RED);
                DrawText(TextFormat("Final Score: %d", score), screenWidth/2 - MeasureText(TextFormat("Final Score: %d", score), 40)/2, screenHeight/2, 40, WHITE);
                
                // Blinking restart text
                if (((int)(GetTime() * 2)) % 2 == 0) {
                    DrawText("Press ENTER to restart", screenWidth/2 - MeasureText("Press ENTER to restart", 20)/2, screenHeight/2 + 80, 20, LIGHTGRAY);
                }
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
