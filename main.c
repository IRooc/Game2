#include "libs/raylib.h"

#define ROWSIZE 10
#define ROWCOUNT 20
typedef struct Level Level;

int windowWidth = 900;
int windowHeight = 600;
int cellWidth = 25;
int gameLeft = 0;
int gameTop = 0;
float dt = 0;
double gameTime = 0;

typedef enum BlockRotation {
  BlockRotation_UP,
  BlockRotation_RIGHT,
  BlockRotation_DOWN,
  BlockRotation_LEFT,
  BlockRotation_COUNT
} BlockRotation;

typedef enum BlockType {
    BlockType_NONE,
    BlockType_LINE,
    BlockType_BLOCK,
    BlockType_PIRAMID,
    BlockType_SNAKEA,
    BlockType_SNAKEB,
    BlockType_LA,
    BlockType_LB,
    BlockType_COUNT,
} BlockType;

typedef enum GameState {
    GameState_MENU,
    GameState_GAME,
    GameState_GAMEOVER,
} GameState;

struct Level {
    int cells[ROWCOUNT][ROWSIZE];

    Vector2 blockPosition;
    BlockRotation blockRotation;
    BlockType blockType;
    BlockType nextBlockType;
    GameState gameState;
    float speed;
    float tickTime;
};

Level level = {{},{3,1},BlockRotation_LEFT,BlockType_BLOCK, BlockType_LINE, GameState_MENU, 1.f, 0.f};


void draw_block(BlockType blockType, BlockRotation rotation, Vector2 position) {

    switch(blockType){
        case BlockType_LINE:{
            if (rotation == BlockRotation_RIGHT || rotation == BlockRotation_LEFT) {
                int x = position.x;// - 1 < 0 ? 0 : position.x - 1;
                for(int i = x; i < x + 4; i++){
                    int cellX = gameLeft + (i*cellWidth);
                    int cellY = gameTop + position.y*cellWidth;
                    DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth }), YELLOW);
                }
            } else if (rotation == BlockRotation_UP || rotation == BlockRotation_DOWN) {
                int y = position.y;// - 1 < 0 ? 0 : position.y - 1;
                for(int i = y; i < y + 4; i++){
                    int cellX = gameLeft + position.x*cellWidth;
                    int cellY = gameTop + (i*cellWidth);
                    DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth }), YELLOW);
                }
            } else {
                TraceLog(LOG_ERROR, "SHOULD NOT HAPPEN THIS... ROTATION WAS COUNT!!!!!!!!!!");
            }
        } break;
        case BlockType_BLOCK: {
            int cellX = gameLeft + (position.x*cellWidth);
            int cellY = gameTop + position.y*cellWidth;
            DrawRectangleRec((CLITERAL(Rectangle) { cellX + cellWidth, cellY, cellWidth, cellWidth }), GREEN);
            DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth }), GREEN);
            DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY + cellWidth, cellWidth, cellWidth }), GREEN);
            DrawRectangleRec((CLITERAL(Rectangle) { cellX + cellWidth, cellY + cellWidth, cellWidth, cellWidth }), GREEN);

        } break;
        case BlockType_NONE: {
            //do nothing
        } break;
        default:{
            TraceLog(LOG_WARNING, "BlockType not implemented %i", blockType);
            int cellX = gameLeft + position.x*cellWidth;
            int cellY = gameTop + (position.y*cellWidth);
            DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth }), ORANGE);
        }
    }
}

int current_block_max_right(BlockType blockType, BlockRotation rotation) {
    switch(blockType) {
        case BlockType_LINE: {
            return rotation == BlockRotation_UP || rotation == BlockRotation_DOWN ? ROWSIZE : ROWSIZE - 3;
        } break;
        case BlockType_BLOCK: {
            return ROWSIZE - 1;
        } break;
        default: {
            return ROWSIZE;
        } break;
    }
}

int current_block_max_down(BlockType blockType, BlockRotation rotation) {
    switch(blockType) {
        case BlockType_LINE: {
            int y = rotation == BlockRotation_UP || rotation == BlockRotation_DOWN ? ROWCOUNT - 3 : ROWCOUNT;
            if (y >= ROWCOUNT) y = ROWCOUNT;
            return y;
        } break;
        case BlockType_BLOCK: {
            return ROWCOUNT - 1;
        } break;
        default: {
            return ROWCOUNT;
        } break;
    }
}

bool current_block_hit(BlockType blockType, BlockRotation rotation, Vector2 position) {
    bool result = false;
    int maxY = current_block_max_down(blockType, rotation);
    switch(blockType){
        case BlockType_LINE: {
            int y = position.y;
            if (y >= maxY) y = maxY;

            int x = (int)position.x;
            TraceLog(LOG_INFO, "x %i y %i maxy %i", x, y, maxY);
            if ((rotation == BlockRotation_UP || rotation == BlockRotation_DOWN)
                &&  (level.cells[y+3][x] != BlockType_NONE || y == maxY)) {
                    level.cells[y-1][x] = blockType;
                    level.cells[y][x] = blockType;
                    level.cells[y+1][x] = blockType;
                    level.cells[y+2][x] = blockType;
                    result = true;
            }
            else if ((rotation == BlockRotation_LEFT || rotation == BlockRotation_RIGHT)
                &&  (
                    level.cells[y][x] != BlockType_NONE ||
                    level.cells[y][x+1] != BlockType_NONE ||
                    level.cells[y][x+2] != BlockType_NONE ||
                    level.cells[y][x+3] != BlockType_NONE ||
                    y == maxY)) {
                level.cells[y-1][x] = blockType;
                level.cells[y-1][x+1] = blockType;
                level.cells[y-1][x+2] = blockType;
                level.cells[y-1][x+3] = blockType;
                result = true;
            }
        } break;
        case BlockType_BLOCK: {
            int y = position.y;
            if (y >= maxY) y = maxY;
            int x = (int)position.x;
            TraceLog(LOG_INFO, "x %i y %i maxy %i", x, y, maxY);
            if (y == maxY ||
                level.cells[y+1][x] != BlockType_NONE ||
                level.cells[y+1][x+1] != BlockType_NONE
                ) {
                    level.cells[y-1][x] = blockType;
                    level.cells[y-1][x+1] = blockType;
                    level.cells[y][x] = blockType;
                    level.cells[y][x+1] = blockType;
                    result = true;
            }
        }
        default: {
            if (position.y > ROWCOUNT) {
                result = true;
            }
        }break;
    }
    return result;
}

void game_step() {
    switch(level.gameState) {
        case GameState_GAMEOVER:
        case GameState_MENU: {
            if (IsKeyPressed(KEY_SPACE)) {
                //Memset should also just work, but not including it yet..
                for(int y = 0; y < ROWCOUNT; y++) {
                    for(int x = 0; x < ROWSIZE; x++) {
                        level.cells[y][x] = BlockType_NONE;
                    }
                }
                level.gameState = GameState_GAME;
                level.blockPosition = (CLITERAL(Vector2){3,1});
                level.blockType = level.nextBlockType;(BlockType)GetRandomValue(BlockType_NONE+1, BlockType_BLOCK);
                level.nextBlockType = (BlockType)GetRandomValue(BlockType_NONE+1, BlockType_BLOCK);
            }
        } break;
        case GameState_GAME: {
            if (IsKeyPressed(KEY_W)) {
                level.blockRotation = (level.blockRotation - 1) < 0 ? BlockRotation_COUNT - 1 : level.blockRotation - 1;
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A)) {
                level.blockPosition.x -= 1;
                if (level.blockPosition.x < 0) level.blockPosition.x = 0;
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) {
                level.blockPosition.x += 1;
                int extreme = current_block_max_right(level.blockType, level.blockRotation);
                if (level.blockPosition.x >= extreme) level.blockPosition.x = extreme - 1;
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S)) {
                level.tickTime -= level.speed;
            }
            if ((level.tickTime + level.speed) < gameTime) {
                level.tickTime = gameTime;
                level.blockPosition.y += 1;
            }

            if (current_block_hit(level.blockType, level.blockRotation, level.blockPosition)) {
                level.blockPosition = (CLITERAL(Vector2){3,1});
                level.blockType = level.nextBlockType;
                level.nextBlockType = (BlockType)GetRandomValue(BlockType_NONE+1, BlockType_BLOCK);
            }

            for(int x = 0; x < ROWSIZE; x++)  {
                if (level.cells[0][x] != BlockType_NONE) {
                    level.gameState = GameState_GAMEOVER;
                }
            }
        } break;
    }

}

void game_draw(){
    BeginDrawing();
    ClearBackground(DARKGRAY);

    //draw the playarea
    float ratio = (float)ROWSIZE/(float)ROWCOUNT;
    int height = (int)(windowHeight*0.9f);
    int width = (int)(height*ratio);
    cellWidth = width/(float)ROWSIZE;
    width = width - width%cellWidth;
    height = height - height%cellWidth;
    gameLeft = windowWidth/4 - width/2;
    gameTop = (windowHeight - height)/2;
    DrawRectangleLinesEx((CLITERAL(Rectangle) { gameLeft - 4, gameTop - 4, width+8, height+8 }), 4.0f, GREEN);

    //draw the smaller rectangles
    for (int y = 0; y < ROWCOUNT; y++) {
        int cellY = gameTop + y*cellWidth;
        for(int x = 0; x < ROWSIZE; x++) {
            int cellX = gameLeft + x*cellWidth;
            Rectangle cellRect = (CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth });

            if (level.gameState != GameState_MENU) {
                switch(level.cells[y][x]) {
                    case BlockType_LINE: {
                        DrawRectangleRec(cellRect, YELLOW);
                    } break;
                    case BlockType_BLOCK: {
                        DrawRectangleRec(cellRect, GREEN);
                    } break;
                }
            }
            DrawRectangleLinesEx(cellRect, 1.0f, GRAY);
        }
    }


    if (level.gameState == GameState_GAME) {
        //draw currentblock
        draw_block(level.blockType, level.blockRotation, level.blockPosition);
    }
    else {
        if (level.gameState == GameState_GAMEOVER) {
            DrawTextEx(GetFontDefault(), "GAME OVER", (CLITERAL(Vector2){20, 50}), 10, 10.0f, RED);
        }
        DrawTextEx(GetFontDefault(), "Press space to begin", (CLITERAL(Vector2){20, 100}), 10, 10.0f, WHITE);
    }



    EndDrawing();
}

int main(void) {
    //allow resize and running minimized
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(windowWidth,windowHeight, "Game 2");
    SetExitKey(KEY_NULL);
    SetWindowMinSize(700,600);
    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        dt = GetFrameTime();
        gameTime += dt;
        windowWidth = GetScreenWidth();
        windowHeight = GetScreenHeight();

        game_step();

        game_draw();
    }

    CloseWindow();

    TraceLog(LOG_INFO, "thank you\n");
}