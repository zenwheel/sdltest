#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define BASE_FRAME_RATE_MS 16

int main(int argc, char **argv) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	if(TTF_Init() < 0) {
		fprintf(stderr, "Couldn't initialize font rendering: %s\n", TTF_GetError());
		return -1;
	}

  SDL_DisplayMode current;
  int width = 800;
  int height = 600;
  for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i){
    if(SDL_GetCurrentDisplayMode(i, &current) != 0)
			fprintf(stderr, "Couldn't get display mode: %s\n", SDL_GetError());
    else {
      printf("Display %d is %dx%d\n", i, current.w, current.h);
      width = current.w;
      height = current.h;
    }
  }

	Uint32 flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	if(argc > 1 && strcmp(argv[1], "-novsync"))
		flags &= ~SDL_RENDERER_PRESENTVSYNC;

	SDL_Window *window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, flags);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_DisableScreenSaver();

	SDL_RendererInfo info;
	SDL_bool vsync = SDL_FALSE;
	if(SDL_GetRendererInfo(renderer, &info) == 0) {
		printf("Using video driver: %s with renderer %s\n", SDL_GetCurrentVideoDriver(), info.name);
		if(info.flags & SDL_RENDERER_SOFTWARE)
			printf("*** Using SDL_RENDERER_SOFTWARE\n");
		if(info.flags & SDL_RENDERER_ACCELERATED)
			printf("*** Using SDL_RENDERER_ACCELERATED\n");
		if(info.flags & SDL_RENDERER_PRESENTVSYNC) {
			printf("*** Using SDL_RENDERER_PRESENTVSYNC\n");
			vsync = SDL_TRUE;
		}
		if(info.flags & SDL_RENDERER_TARGETTEXTURE)
			printf("*** Using SDL_RENDERER_TARGETTEXTURE\n");
	}

	TTF_Font *font = TTF_OpenFont("AlteHaasGroteskBold.ttf", 60);
	if(font == 0) {
		fprintf(stderr, "Can't open font for stats: %s", TTF_GetError());
		goto cleanup;
	}

	SDL_Event event;
	SDL_bool m_run = SDL_TRUE;
	Uint32 lastFrame = 0;
	float fps = 0.0f;
	SDL_Color foregroundColor = { 0xff, 0xff, 0 };
	char tmp[64];
	int delta = 1;
	int position = 0;
	while(m_run) {
		while(SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_ESCAPE)
				m_run = SDL_FALSE;
				break;
			case SDL_QUIT:
				m_run = SDL_FALSE;
				break;
			default:
				break;
			}
		}

		Uint32 start = SDL_GetTicks();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
		SDL_RenderClear(renderer);

		snprintf(tmp, sizeof(tmp), "Current fps = %.1f", fps);
		SDL_Surface *textSurface = TTF_RenderText_Solid(font, tmp, foregroundColor);

		if(textSurface) {
			SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

			SDL_Rect location;
			location.h = textSurface->h;
			location.w = textSurface->w;
			location.x = position;
			location.y = height / 2 - textSurface->h / 2;
			SDL_FreeSurface(textSurface);

			position += delta;
			if(position >= width - textSurface->w || position <= 0)
				delta *= -1;

			if(textTexture) {
				SDL_RenderCopy(renderer, textTexture, 0, &location);
				SDL_DestroyTexture(textTexture);
			}
		}

		SDL_RenderPresent(renderer);

		Uint32 end = SDL_GetTicks();
		Uint32 elapsed = end - lastFrame;

		// delay for the remainder of the base rate so we keep a decent frame rate if there's no vsync
		if(vsync == SDL_FALSE && BASE_FRAME_RATE_MS > elapsed)
			SDL_Delay(BASE_FRAME_RATE_MS - elapsed);

		// remember now as the starting point for the next frame
		lastFrame = SDL_GetTicks();

		// update statistics
		elapsed = lastFrame - start;
		fps = 1000.0f / (float)elapsed;
		printf("Current frame rate is %6.3f fps (%ums)              \r", fps, elapsed);
	}
	printf("\n");

cleanup:
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();

	return 0;
}
