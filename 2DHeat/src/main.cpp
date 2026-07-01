#include <iostream>
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <vector>

constexpr int WINDOW_WIDTH = 900;
constexpr int WINDOW_HEIGHT = 600;

constexpr int MAX_CG_ITERATIONS = 80;
constexpr float CG_TOLERANCE = 1e-5f;

constexpr int RADIUS = 10;

std::vector<float> Temp_new(WINDOW_WIDTH * WINDOW_HEIGHT);
std::vector<float> Temp_old(WINDOW_WIDTH * WINDOW_HEIGHT);

// CG scratch buffers
std::vector<float> CG_Residual(WINDOW_WIDTH * WINDOW_HEIGHT);
std::vector<float> CG_direction(WINDOW_WIDTH * WINDOW_HEIGHT);
std::vector<float> CG_Adirection(WINDOW_WIDTH * WINDOW_HEIGHT);
std::vector<float> CG_Ax(WINDOW_WIDTH * WINDOW_HEIGHT);

//CG preconditioner buffer
std::vector<float> CG_z(WINDOW_WIDTH * WINDOW_HEIGHT);

std::vector<Color> pixels(WINDOW_WIDTH * WINDOW_HEIGHT);

inline int ID(int x, int y)
{
    return y * WINDOW_WIDTH + x;
}

struct HeatParams
{
    float kappa = 120.0f;   // diffusion strength
    float h = 1.0f;       // grid cell size
    float dt = 0.1f;      // timestep

    float Alpha() const
    {
        return kappa * dt / (h * h);
    }
};

void applyA(
    const std::vector<float>& x,
    std::vector<float>& y,
    float r
) {
    std::fill(y.begin(), y.end(), 0.0f);

    for (int j = 1; j < WINDOW_HEIGHT - 1; j++) {
        for (int i = 1; i < WINDOW_WIDTH - 1; i++) {
            int p = ID(i, j);

            y[p] =
                (1.0f + 4.0f * r) * x[p]
              - r * x[p + 1]
              - r * x[p - 1]
              - r * x[p + WINDOW_WIDTH]
              - r * x[p - WINDOW_WIDTH];
        }
    }
}

float dotInterior(const std::vector<float>& x, const std::vector<float>& y)
{
    float d = 0;
    for (int j = 1; j < WINDOW_HEIGHT - 1; j++)
    {
        for (int i = 1; i < WINDOW_WIDTH - 1; i++) 
        {
            int p = j * WINDOW_WIDTH + i;
            d += x[p] * y[p];
        }
    }

    return d;
}

void SolveCG(
    std::vector<float>& T_n,
    std::vector<float>& T_old,
    std::vector<float>& r,
    std::vector<float>& d,
    std::vector<float>& A_d,
    std::vector<float>& A_x,
    float alpha
) {

    applyA(T_n, A_x, alpha);

    const float invDiag = 1.0f / (1.0f + 4.0f * alpha);

    for (int j = 1; j < WINDOW_HEIGHT - 1; j++) {
        for (int i = 1; i < WINDOW_WIDTH - 1; i++) {
            int p = j * WINDOW_WIDTH + i;

            r[p] = T_old[p] - A_x[p];
            d[p] = invDiag*r[p];
        }
    }

    float rsOld = invDiag * dotInterior(r, r);

    if (std::sqrt(rsOld) < CG_TOLERANCE) {
        return;
    }

    for (int iter = 0; iter < MAX_CG_ITERATIONS; iter++) {
        applyA(d, A_d, alpha);

        float denom =  dotInterior(d, A_d);

        if (std::abs(denom) < 1e-20f) {
            return;
        }

        float alphaCG = rsOld / denom;

        for (int j = 1; j < WINDOW_HEIGHT - 1; j++) {
            for (int i = 1; i < WINDOW_WIDTH - 1; i++) {
                int p = j * WINDOW_WIDTH + i;

                T_n[p] += alphaCG * d[p];
                r[p] -= alphaCG * A_d[p];
            }
        }

        float rsNew = invDiag * dotInterior(r, r);

        if (std::sqrt(dotInterior(r, r)) < CG_TOLERANCE) {
                std::cout << "Iterations"<< iter <<std::endl;
            return;
        }

        float beta = rsNew / rsOld;

        for (int j = 1; j < WINDOW_HEIGHT - 1; j++) {
            for (int i = 1; i < WINDOW_WIDTH - 1; i++) {
                int p = j * WINDOW_WIDTH + i;

                d[p] = invDiag * r[p] + beta * d[p];
            }
        }

        rsOld = rsNew;
    }

    std::cout << "Max iteration reached"<<std::endl;
    return;
}

void ApplyBoundary(std::vector<float>& T)
{
    for (int x = 0; x < WINDOW_WIDTH; x++)
    {
        T[ID(x, 0)] = 0.0f;
        T[ID(x, WINDOW_HEIGHT - 1)] = 0.0f;
    }

    for (int y = 0; y < WINDOW_HEIGHT; y++)
    {
        T[ID(0, y)] = 0.0f;
        T[ID(WINDOW_WIDTH - 1, y)] = 0.0f;
    }
}

void SimulationStep(
    std::vector<float>& t_n,
    std::vector<float>& t_old,
    std::vector<float>& r,
    std::vector<float>& d,
    std::vector<float>& ad,
    std::vector<float>& ax,
    HeatParams params
) {
    ApplyBoundary(t_old);

    t_n = t_old;

    ApplyBoundary(t_n);

    SolveCG(t_n, t_old, r, d, ad, ax, params.Alpha());

    t_old = t_n;
    ApplyBoundary(t_n);
}

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "2D Heat Sim");

    Image image = GenImageColor(WINDOW_WIDTH, WINDOW_HEIGHT, WHITE);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    HeatParams params{};

    while(!WindowShouldClose())
    {
        
        Vector2 mouse = GetMousePosition();
        bool leftClick = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        
        if(leftClick)
        {
            const int x = std::clamp<int>(mouse.x, 0, WINDOW_WIDTH-1);
            const int y = std::clamp<int>(mouse.y, 0, WINDOW_HEIGHT - 1);

            for (int i = 0; i < 2 * RADIUS;i++)
            {
                for (int j = 0; j < 2 * RADIUS; j++)
                {
                    if(y+j-RADIUS <0 || y+j-RADIUS >= WINDOW_HEIGHT)
                    {
                        continue;
                    }

                    if(x+i-RADIUS <0 || x+i-RADIUS >= WINDOW_WIDTH)
                    {
                        continue;
                    }
                    
                    if((i-RADIUS)*(i-RADIUS)+ (j-RADIUS)*(j-RADIUS) < RADIUS*RADIUS)
                    {
                        Temp_old[ID(x+i-RADIUS,y+j-RADIUS)] += 0.1f;
                        Temp_old[ID(x+i-RADIUS,y+j-RADIUS)] = std::min(1.0f, Temp_old[ID(x+i-RADIUS,y+j-RADIUS)]);
                    }
                }
            }            
            std::cout << "click at" << x << " " << y << " temp ="<< Temp_old[ID(x,y)]<<std::endl;
        }

        SimulationStep(Temp_new,Temp_old,CG_Residual,CG_direction,CG_Adirection,CG_Ax,params);

        for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++)
        {
            float rawValue = Temp_new[i] * 255.0f;
            unsigned char colorOffset =
                static_cast<unsigned char>(std::clamp(rawValue, 0.0f, 255.0f));

            pixels[i] = Color{ colorOffset, 0, 0, 255 };
        }

        UpdateTexture(texture, pixels.data());


        BeginDrawing();
        ClearBackground(WHITE);
        DrawTexture(texture, 0, 0, WHITE);
        DrawText(TextFormat("FPS: %d", GetFPS()), 20, 20, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(texture);

    CloseWindow();
    return 0;
}