#include <boost/di.hpp>

#include "app/App.h"

namespace di = boost::di;

static std::shared_ptr<App> app;

void setup() {
    auto injector = di::make_injector();
    app = injector.create<std::shared_ptr<App>>();
    app->setup();
}

void loop() {
    app->loop();
}
