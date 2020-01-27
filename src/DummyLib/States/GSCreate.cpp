#include "GSCreate.h"

#include <stdexcept>

#include "GSDrawTest.h"
#include "GSUITest.h"
#include "GSUITest2.h"
#include "GSUITest3.h"

std::shared_ptr<GameState> GSCreate(eGameState state, GameBase *game) {
    if (state == GS_DRAW_TEST) {
        return std::make_shared<GSDrawTest>(game);
    } else if (state == GS_UI_TEST) {
        return std::make_shared<GSUITest>(game);
    } else if (state == GS_UI_TEST2) {
        return std::make_shared<GSUITest2>(game);
    } else if (state == GS_UI_TEST3) {
        return std::make_shared<GSUITest3>(game);
    }
    throw std::invalid_argument("Unknown game state!");
}