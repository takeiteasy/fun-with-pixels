#include "pb.h"
#include "rng.h"
#include <assert.h>

int main(int argc, const char *argv[]) {
    assert(pbBegin(640, 480, "test", 0));
    pbImage *test = pbImageNew(640, 480);
    while (pbPoll()) {
        pbImageFill(test, RGB(0, 0, 255));
        pbFlush(test);
    }
    pbEnd();
    return 0;
}
