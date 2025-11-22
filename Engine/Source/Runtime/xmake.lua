target_rules("NilouEditor", "Editor")
    add_deps("Launch")
    add_files("./NilouEditor.cpp")

target_rules("NilouGame", "Game")
    add_deps("Launch")
    add_files("./NilouEditor.cpp")