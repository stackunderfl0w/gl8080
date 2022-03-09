#include <iostream>

#ifdef __ANDROID__
#include <SDL.h>
#include <SDL_opengles2.h>
#include <android/log.h>
#include "glhelpers/shaders_gles.h"
#else
#include <glad/glad.h>
//#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include "glhelpers/shaders.h"
#endif

#ifdef NINTENDO_SWITCH
#include <switch.h>
#endif


#include <array>
#include <chrono>
#include <glm/vec3.hpp>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl.h"
#include "imgui/backends/imgui_impl_opengl3.h"
//#include "tinyexpr/tinyexpr.h"

#include "platform.h"
#define STB_IMAGE_IMPLEMENTATION

#include "glhelpers/shaderloader.h"
#include "glhelpers/textureloader.h"
#include "glhelpers/shader.h"

#include "../zip/src/zip.c"
#include "i8080.cpp"


typedef std::chrono::high_resolution_clock Clock;
#define systime chrono::duration_cast<std::chrono::microseconds>(chrono::high_resolution_clock::now()-starttime).count()
int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

int sdl_interval_mode=1;

struct pix{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
}__attribute__ ((packed));

using namespace std;
int on_extract_entry(const char *filename, void *arg) {
    static int i = 0;
    int n = *(int *)arg;
    printf("Extracted: %s (%d of %d)\n", filename, ++i, n);

    return 0;
}
double round(double value, int places) {
    if (places < 0) {return 0;};

    long factor = (long) pow(10, places);
    value = value * factor;
    long tmp = (long)(value);
    return (double)tmp / factor;
}
void load_file(const char *filename,unsigned char *mem, int adr){
    FILE *file = fopen(filename,"rb");
    if (file != NULL)
    {
        cout<<endl<<"File opened "<<filename<<endl;
    }
    fseek(file, 0L, SEEK_END);
    int fsize = ftell(file);
    fseek(file, 0L, SEEK_SET);
    unsigned char *buffer = (unsigned char*) malloc(fsize);
    fread(buffer, fsize, 1, file);
    fclose(file);
    cout<<dec<<"     "<<fsize<<" bytes loaded"<<endl;
    //cout<<"     "<<"Header-"<<buffer<<endl; //removed, causes windows alert sound with corrupted headers
    for (int i = 0; i < fsize; ++i){
        mem[i+adr] = buffer[i];
        // Print for debug purposes
        //printf("%02X", memory[0x200 + i]);
        //cout<<endl;
    }
}
void render(unsigned char* disp, pix* tex){
    int width=224;
    int height=256;
    memset(tex,0,sizeof(pix)*width*height);

    for (int y = 0; y < 224; y++) {
        for (int x = 0; x < 32; x++) {
            for (int z = 0; z<8;z++){
                if((disp[0x2400+32*y+x]>>z)%2==1){
                    //SDL_Rect r={y, 256-(x*8+z), 1, 1};
                    //SDL_RenderFillRect( renderer, &r );
                    int newx = (int) (y * width / 224.0);
                    int newy = (int) ((256-(x*8+z)) * height / 256.0);
                    //int pixelwidth = (int) (((y + 1) * width / 224.0) - newx);
                    //int pixelheight = (int) ((((256-(x*8+z)) + 1) * height / 256.0) - newy);
                    //SDL_Rect r={newx, newy, pixelwidth, pixelheight};
                    //SDL_RenderFillRect( renderer, &r );
                    tex[((x*8+z))*width+y].r=255;
                    tex[((x*8+z))*width+y].g=255;
                    tex[((x*8+z))*width+y].b=255;
                    //tex[((x*8+z))*width+y].a=255;
                    //cout<<"hi"<<endl;

                }
            }
        }
    }
}
int main(int argc, char *argv[]) {
    std::cout << "Hello, World!" << std::endl;

    if(argv[1]!=NULL){
        cout<<argv[1]<<endl;
    }
    else{
        //return 1;
    }

    platform::init();
    SDL_Window *window=platform::get_window();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    int w,h;
    //GLuint tex = loadtexture(argv[1],true,&w,&h);

    //GLuint tex = loadtexture(argv[1], true, pixels, &w, &h);
    //GLuint tex = loadtexture("desktop.png", true, pixels, &w, &h);
    GLuint tex;// = loadtexture("desktop.png", true, pixels, &w, &h);
    GLuint mem;// = loadtexture("desktop.png", true, pixels, &w, &h);
    int width, height, nrChanels;
    width=224;
    height=256;
    pix* pixels= (pix *)(malloc(sizeof(pix) * width * height));
    if(pixels != nullptr)
    {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }


    /*for (int i = 0; i < 10000; ++i) {
        pixels[i].r=255;
        pixels[i].g=255;
        pixels[i].b=255;
        pixels[i].a=255;
    }*/
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    cout<<w<<" "<<h<<endl;
    int v_comp,f_comp;
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS,&v_comp);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,&f_comp);
    cout<<v_comp<<endl;
    cout<<f_comp<<endl;
    float aspect_ratio=(float)w/(float)h;

    auto starttime = Clock::now();

    //platform::set_title("hello");
    //glEnable(GL_DEPTH_TEST);
    //glEnable(GL_CULL_FACE);

    unsigned long long deltatime;
    double fps;
    bool mouse_down = false;
    float ddpi, hdpi, vdpi;
    SDL_GetDisplayDPI(0,&ddpi,&hdpi,&vdpi);

#ifndef __ANDROID__
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
#endif

    //Shader uiShader(vertex2d,fragment2d);
    Shader uiShader(vertex2d,invadersfragment_old);
    uiShader.use();
    //GLuint tex = loadtexture_no_filter("2021-05-06 10_42_34-Meet - gfu-ebue-qda.png",true);
    //GLuint tex = loadtexture_transparent_no_filter("crosshair.png",false);

    float quadCoords[] = {
            -1.0f, -1.0f,  // bottom left
            1.0f, -1.0f,  // bottom right
            1.0f,  1.0f,  // top right
            1.0f,  1.0f,  // top right
            -1.0f,  1.0f,   // top left
            -1.0f, -1.0f // bottom left

    };
    float texCoords[] = {
            0.0f, 0.0f, // bottom left
            1.0f, 0.0f, // bottom right
            1.0f, 1.0f, // top right
            1.0f, 1.0f, // top right
            0.0f, 1.0f, // top left
            0.0f, 0.0f // bottom left

    };
    GLuint vertexbuffer;
    GLuint uvbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadCoords)* sizeof(float) , &quadCoords[0], GL_DYNAMIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)* sizeof(float) , &vertices[0], GL_DYNAMIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords)* sizeof(float) , &texCoords[0], GL_DYNAMIC_DRAW);


    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

    long long last_fps_update = 0;//systime
    long long last_frame_render = 0;//systime
    long long next_frame=0;
    long long last_frame[60];
    float deltaTime = 0.0f; // time between current frame and last frame
    float lastFrame = 0.0f;

    int max_fps=60;
    int clock_speed=2000000;
    int clocks_per_frame=clock_speed/max_fps;

    SDL_Event windowEvent;
    bool captureinput=false;
    float x=0,y=0;
    float scale=1.0;
    bool refresh=true;

    i8080 cpu;

    //Screen screen;

    SDL_Event e;
    bool running = true;

    glGenTextures(1, &mem);
    glBindTexture(GL_TEXTURE_2D, mem);

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 7168, height, 0, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(mem));
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);



    int arg = 2;
    if(argv[1]!=NULL){
        zip_extract(argv[1], "tmp", on_extract_entry, &arg);
    }
    //SDL_Delay(1000);
    load_file("tmp/invaders.h",cpu.memory,0x0000);
    load_file("tmp/invaders.g",cpu.memory,0x0800);
    load_file("tmp/invaders.f",cpu.memory,0x1000);
    load_file("tmp/invaders.e",cpu.memory,0x1800);
#if defined _WIN32 || defined _WIN64
    system( "rmdir /s /q tmp");
#else
    system( "rm -rf ./tmp");
#endif

    last_frame[0]=5;

    while (running){
        SDL_GetWindowSize(window, &SCR_WIDTH, &SCR_HEIGHT);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        float ratio = (float)max(SCR_WIDTH,SCR_HEIGHT)/ (float)min(SCR_WIDTH,SCR_HEIGHT);
        long long framestart = systime;
        deltatime = (systime - last_frame[0]);
        float currentFrame = SDL_GetTicks()/1000.0;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //calc fps
        if(true){
            next_frame=systime+1000000/(max_fps);
            //next_frame = systime + 1000000 / (max_fps);
            fps = (1000000.0 / (systime - last_frame[59]) * 60);
            //cout<<systime - last_fps_update<<endl;
            if (systime - last_fps_update > 500000) {
                last_fps_update = systime;
                platform::set_title(to_string(fps).c_str());
                //cout<<to_string(fps).c_str()<<endl;
            }
            for (int i = 59; i > 0; --i) {
                last_frame[i] = last_frame[i - 1];
            }
            last_frame[0] = systime;
        }
        while(SDL_PollEvent(&windowEvent)) {
            ImGui_ImplSDL2_ProcessEvent(&windowEvent);
            if (windowEvent.type == SDL_QUIT) {
                running = false;
            }
            if (windowEvent.type == SDL_KEYDOWN){
                //running = false;
                switch(windowEvent.key.keysym.sym){
                    //case SDLK_1:chip8.keys[0]=true;break;
                    case SDLK_SPACE:cpu.player_one_fire=true;break;
                    case SDLK_a:cpu.player_one_left=true;break;
                    case SDLK_d:cpu.player_one_right=true;break;
                    case SDLK_o:cpu.one_player_button=true;break;
                    case SDLK_c:cpu.coin=true;break;
                    case SDLK_INSERT:cpu.player_two_fire=true;break;
                    case SDLK_LEFT:cpu.player_two_left=true;break;
                    case SDLK_RIGHT:cpu.player_two_right=true;break;
                    case SDLK_p:cpu.two_player_button=true;break;
                    case SDLK_f:running=false;break;
                }

            }
            if (windowEvent.type == SDL_KEYUP){
                switch(windowEvent.key.keysym.sym){
                    //case SDLK_1:chip8.keys[0]=false;break;
                    case SDLK_SPACE:cpu.player_one_fire=false;break;
                    case SDLK_a:cpu.player_one_left=false;break;
                    case SDLK_d:cpu.player_one_right=false;break;
                    case SDLK_o:cpu.one_player_button=false;break;
                    case SDLK_c:cpu.coin=false;break;
                    case SDLK_INSERT:cpu.player_two_fire=false;break;
                    case SDLK_LEFT:cpu.player_two_left=false;break;
                    case SDLK_RIGHT:cpu.player_two_right=false;break;
                    case SDLK_p:cpu.two_player_button=false;break;
                }

            }
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE){
                running=false;
            }

            if (windowEvent.type == SDL_KEYDOWN &&
                windowEvent.key.keysym.sym == SDLK_TAB){
                captureinput=!captureinput;
                if(captureinput){
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    //SDL_SetWindowGrab(window, SDL_TRUE);

                }
                else{
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                    //SDL_SetWindowGrab(window, SDL_FALSE);
                }

            }
            if(!io.WantCaptureMouse){
                if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                    if(windowEvent.type == SDL_MOUSEMOTION){
                        refresh=true;
                        x+=1/scale*windowEvent.motion.xrel/SCR_WIDTH*2;
                        y-=1/scale*windowEvent.motion.yrel/SCR_HEIGHT*2;
                    }
                }

                if(windowEvent.type == SDL_MOUSEWHEEL){
                    refresh=true;
                    if(windowEvent.wheel.y>0){
                        scale*=.9;
                    } else{
                        scale/=.9;
                    }
                }
            }
        }
        auto start = chrono::high_resolution_clock::now();

        while(cpu.cycles<clocks_per_frame/2){
            cpu.cycle();
        }
        cpu.cycles-=clocks_per_frame/2;
        cpu.run_innterrupt(0xcf);
        while(cpu.cycles<clocks_per_frame/2){
            cpu.cycle();
        }
        cpu.cycles-=clocks_per_frame/2;
        cpu.run_innterrupt(0xd7);

        //for (int i = 0; i < 50000; ++i){cpu.cycle();}
        if ((systime-last_frame_render)/1000>16){
            //render(cpu.memory,pixels);
            //glBindTexture(GL_TEXTURE_2D, tex);
            last_frame_render=systime;
        }
        //thread thread_obj(screen.render,cpu.memory);

        auto elapsed = chrono::high_resolution_clock::now() - start;
        long long microseconds = chrono::duration_cast<std::chrono::microseconds>(elapsed).count();



        uiShader.use();


        glBindVertexArray(VertexArrayID);//fuck i have to do this shit
        //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


        //glUniform1fv(glGetUniformLocation(program, "v"), 10, v);
        glUniform1iv(glGetUniformLocation(uiShader.ID,"disp"),0x1c00, reinterpret_cast<const GLint *>(&cpu.memory[0x2400]));
        //glUniform1iv(glGetUniformLocation(uiShader.ID,"disp2"),0x1c00-1024, reinterpret_cast<const GLint *>(&cpu.memory[0x2400+4096]));

       // glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, mem);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1000,1, 0, GL_R, GL_UNSIGNED_BYTE, reinterpret_cast<const void *>(cpu.memory[0x2400]));

        uiShader.setInt("mem",0);

        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);



        //glBindVertexArray(0);//fuck i have to do this shit

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
                2,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
        );
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                1,                                // attribute
                2,                                // size
                GL_FLOAT,                         // type
                GL_FALSE,                         // normalized?
                0,                                // stride
                (void*)0                          // array buffer offset
        );
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 12*3 indices starting at 0 -> 12 triangles


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
        //ImGui::Text("Position = %f %f %f", camera.Position.x, camera.Position.y,camera.Position.z);

        ImGui::SliderFloat("scale", &scale,.5,2.0);
        ImGui::SliderFloat("x", &x,-1.0,1.0);
        ImGui::SliderFloat("y", &y,-1.0,1.0);
        if(ImGui::Button("filtering")){
            static bool filtering;
            filtering=!filtering;
            if(filtering){
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
            else{
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }
        }

        ImGui::NewLine();ImGui::NewLine();ImGui::NewLine();

        ImGui::SliderInt("Swap interval",&sdl_interval_mode,-1,3);
        SDL_GL_SetSwapInterval(sdl_interval_mode);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();


        ImGui::Render();

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);


        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
        unsigned int time_left=(next_frame-systime)/1000;
        if (time_left<17){
            //cout<<time_left<<endl;
            SDL_Delay(time_left);
        }
    }
    return 0;
}





