set(requires esp_http_server AP_STA rgb_led touchpad delay)

idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES ${requires}
    EMBED_FILES
        "web/index.html"
        "web/style.css"
        "web/app.js"
)