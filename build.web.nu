(emcc -o solitaire.html 
    main.c -Os -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result
    C:/raylib/raylib/src/lib/web/libraylib.a 
    -I. -I C:/raylib/raylib/src 
    -L. -L C:/raylib/raylib/src/lib/web 
    -s USE_GLFW=3 
    -s ASYNCIFY 
    -s ASSERTIONS  
    --shell-file ./shell.html
    -DPLATFORM_WEB)
