#include "libs/raylib.h"
#include "libs/raymath.h"
#include <string.h>

#define ROWSIZE 10
#define ROWCOUNT 20
#define BLOCK_MOVE_ANIM_LENGTH 0.125f

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

typedef struct Level Level;
struct Level {
    int cells[ROWCOUNT][ROWSIZE];

    Vector2 blockFromPosition;
    Vector2 blockPosition;
    BlockRotation blockRotation;
    BlockType blockType;
    BlockType nextBlockType;
    GameState gameState;

    float speed;
    float tickTime;

    float animBlockMoveDuration;
};

Level level = {};


void draw_block(BlockType blockType, BlockRotation rotation, Vector2 position) {

    switch(blockType){
        case BlockType_LINE:{
            if (rotation == BlockRotation_RIGHT || rotation == BlockRotation_LEFT) {
                for(int i = 0; i < 4; i++){
                    int cellX = gameLeft + ((i + position.x)*cellWidth);
                    int cellY = gameTop + position.y*cellWidth;
                    DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth }), YELLOW);
                }
            } else if (rotation == BlockRotation_UP || rotation == BlockRotation_DOWN) {
                for(int i = 0; i < 4; i++){
                    int cellX = gameLeft + position.x*cellWidth;
                    int cellY = gameTop + ((i + position.y)*cellWidth);
                    DrawRectangleRec((CLITERAL(Rectangle) { cellX, cellY, cellWidth, cellWidth }), YELLOW);
                }
            } else {
                TraceLog(LOG_ERROR, "SHOULD NOT HAPPEN THIS... ROTATION WAS COUNT!!!!!!!!!!");
            }
        } break;
        case BlockType_BLOCK: {
            int cellX = gameLeft + position.x*cellWidth;
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

bool is_move_allowed(BlockType blockType, BlockRotation rotation, Vector2 position, Vector2 move) {
    bool result = true;
    int x = position.x + move.x;
    int y = position.y;
    switch(blockType) {
        case BlockType_LINE: {
            if (rotation == BlockRotation_UP || rotation == BlockRotation_DOWN) {
                if (level.cells[y][x] != BlockType_NONE ||
                    level.cells[y+1][x] != BlockType_NONE ||
                    level.cells[y+2][x] != BlockType_NONE ||
                    level.cells[y+3][x] != BlockType_NONE) {
                    result = false;
                }

            } else if (rotation == BlockRotation_LEFT || rotation == BlockRotation_RIGHT) {
                if (level.cells[y][x] != BlockType_NONE
                    || (move.x > 0.f && level.cells[y][x+3] != BlockType_NONE)) {
                    result = false;
                }
            }
        } break;
        case BlockType_BLOCK: {
            if (move.x > 0.f) {
                if (level.cells[y][x+1] != BlockType_NONE ||
                    level.cells[y+1][x+1] != BlockType_NONE) {
                    result = false;
                }
            }
            else {
                if (level.cells[y][x] != BlockType_NONE ||
                    level.cells[y+1][x] != BlockType_NONE) {
                    result = false;
                }

            }
        } break;

    }
    return result;
}

bool current_block_hit(BlockType blockType, BlockRotation rotation, Vector2 position) {
    bool result = false;
    int maxY = current_block_max_down(blockType, rotation);
    switch(blockType){
        case BlockType_LINE: {
            int y = position.y;
            if (y >= maxY) y = maxY;

            int x = (int)position.x;
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
            if (position.y >= maxY) {
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
                level.tickTime = gameTime;
                level.speed = 1.0f;
                level.blockPosition = (CLITERAL(Vector2){3,1});
                level.blockFromPosition = (CLITERAL(Vector2){3,1});
                level.blockType = BlockType_LINE;// (BlockType)GetRandomValue(BlockType_NONE+1, BlockType_BLOCK);
                level.nextBlockType = (BlockType)GetRandomValue(BlockType_NONE+1, BlockType_BLOCK);
            }
        } break;
        case GameState_GAME: {
            if (IsKeyPressed(KEY_ESCAPE)) {
                //Memset should also just work, but not including it yet..
                for(int y = 0; y < ROWCOUNT; y++) {
                    for(int x = 0; x < ROWSIZE; x++) {
                        level.cells[y][x] = BlockType_NONE;
                    }
                }
                level.gameState = GameState_MENU;
            }
            if (IsKeyPressed(KEY_W)) {
                level.blockRotation = (level.blockRotation - 1) < 0 ? BlockRotation_COUNT - 1 : level.blockRotation - 1;
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressedRepeat(KEY_A)) {
                if (is_move_allowed(level.blockType, level.blockRotation, level.blockPosition, (CLITERAL(Vector2){-1, 0}))) {
                    level.blockPosition.x -= 1;
                    if (level.blockPosition.x < 0) level.blockPosition.x = 0;
                    if (level.animBlockMoveDuration < 0.2f) level.animBlockMoveDuration = 1.0f;
                }
            }
            if (IsKeyPressed(KEY_D) || IsKeyPressedRepeat(KEY_D)) {
                if (is_move_allowed(level.blockType, level.blockRotation, level.blockPosition, (CLITERAL(Vector2){1, 0}))) {
                    level.blockPosition.x += 1;
                    int extreme = current_block_max_right(level.blockType, level.blockRotation);
                    if (level.blockPosition.x >= extreme) level.blockPosition.x = extreme - 1;
                    if (level.animBlockMoveDuration < 0.2f) level.animBlockMoveDuration = 1.0f;
                }
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressedRepeat(KEY_S)) {
                level.tickTime -= level.speed;
            }
            if ((level.tickTime + level.speed) < gameTime) {
                level.tickTime = gameTime;
                level.blockPosition.y += 1;
                level.animBlockMoveDuration = 1.0f;
            }

            if (current_block_hit(level.blockType, level.blockRotation, level.blockPosition)) {
                level.blockPosition = (CLITERAL(Vector2){3,1});
                level.blockFromPosition = level.blockPosition;
                level.blockType = level.nextBlockType;
                level.animBlockMoveDuration = 0.0f;
                level.nextBlockType = (BlockType)GetRandomValue(BlockType_NONE+1, BlockType_BLOCK);
            }

            //check filled lines and remove them
            for(int y = 0; y < ROWCOUNT; y++) {
                bool isfilled = true;
                for(int x = 0; x < ROWSIZE; x++) {
                    if (level.cells[y][x] == BlockType_NONE) {
                        isfilled = false;
                        break;
                    }
                }
                if (isfilled) {
                    for (int r = y; r > 0; r--) {
                        for(int c = 0; c < ROWSIZE; c++) {
                            level.cells[r][c] = level.cells[r-1][c];
                        }
                    }
                    for(int c = 0; c < ROWSIZE; c++) {
                        level.cells[0][c] = 0;
                    }
                }
            }


            //check endstate
            //NOTE(rc):  might just check if the blocktype fits, but this is ok for now
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
        draw_block(level.blockType, level.blockRotation, level.blockFromPosition);
        //draw next block
        int cellX = gameLeft + 11.5f*cellWidth;
        int cellY = gameTop + 6*cellWidth;
        DrawTextEx(GetFontDefault(), "NEXT BLOCK", (CLITERAL(Vector2){ cellX, cellY}), windowWidth / 60.f, 10.f, RED);
        draw_block(level.nextBlockType, BlockRotation_UP, (CLITERAL(Vector2){14, 7 }));
    } else {
        float fontBase = windowWidth / 20.f;
        if (level.gameState == GameState_GAMEOVER) {
            DrawTextEx(GetFontDefault(), "GAME OVER", (CLITERAL(Vector2){20, (fontBase * 1.5)}), fontBase, 10.0f, RED);
        }
        DrawTextEx(GetFontDefault(), "Press space to begin", (CLITERAL(Vector2){20, (fontBase * 1.5)*2}), fontBase, 10.0f, WHITE);
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
        level.animBlockMoveDuration = (level.animBlockMoveDuration*BLOCK_MOVE_ANIM_LENGTH - dt)/BLOCK_MOVE_ANIM_LENGTH;
        if (level.animBlockMoveDuration <= 0.f) level.animBlockMoveDuration = 0.f;
        else {
            level.blockFromPosition = Vector2Lerp(level.blockFromPosition, level.blockPosition, (1.0f - level.animBlockMoveDuration));
        }
        windowWidth = GetScreenWidth();
        windowHeight = GetScreenHeight();

        game_step();

        game_draw();
    }

    CloseWindow();

    TraceLog(LOG_INFO, "thank you\n");
}
