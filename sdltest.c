#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/timeb.h>
#include <SDL.h>
#include <SDL_ttf.h>

#define BASE_FRAME_RATE_MS 16

#define DRI_PATH "/proc/dri/0/name"

int GetIntelOffset() {
  int result = 0;
  struct stat statbuf;
  int res = stat(DRI_PATH, &statbuf);
  if(res == -1 || !S_ISREG(statbuf.st_mode))
    return result;

  FILE *f = fopen(DRI_PATH, "r");
  if(f) {
    char buf[255];
    if(fgets(buf, sizeof(buf), f)) {
      char *p = strchr(buf, ' ');
      if(p) *p = 0;
      printf("Graphics hardware is '%s'\n", buf);
      if(!strcmp(buf, "i915"))
        result++;
    }
    fclose(f);
  }

  return result;
}

int main(int argc, char **argv) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	if(TTF_Init() < 0) {
		fprintf(stderr, "Couldn't initialize font rendering: %s\n", TTF_GetError());
		return -1;
	}

  int width = 800;
  int height = 600;
  Uint32 frameRate = BASE_FRAME_RATE_MS;
  SDL_bool vsync = SDL_FALSE;

#if SDL_MAJOR_VERSION == 2
  SDL_DisplayMode current;
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
	if(argc > 1) {
		if(strncmp(argv[1], "-novsync", 8) == 0)
			flags &= ~SDL_RENDERER_PRESENTVSYNC;
		if(strncmp(argv[1], "-software", 9) == 0)
			flags = SDL_RENDERER_SOFTWARE;

		char *n = strchr(argv[1], '=');
		if(n) {
			n++;
			frameRate = strtoul(n, 0, 0);
		}
	}

	SDL_Window *window = SDL_CreateWindow(argv[0], SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width + GetIntelOffset(), height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, flags);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_DisableScreenSaver();

	SDL_RendererInfo info;
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
#else
  const SDL_VideoInfo *pVideoInfo = SDL_GetVideoInfo();
  if(pVideoInfo == 0) {
    fprintf(stderr, "Couldn't get display information: %s\n", SDL_GetError());
    exit(1);
  }
  printf("Display is %dx%d\n", pVideoInfo->current_w, pVideoInfo->current_h);
  width = pVideoInfo->current_w;
  height = pVideoInfo->current_h;

  SDL_Surface *screen = screen = SDL_SetVideoMode(width, height,
    //pVideoInfo->current_w, pVideoInfo->current_h,
    pVideoInfo->vfmt->BitsPerPixel, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
  if(screen == 0) {
    fprintf(stderr, "Couldn't set video mode: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_ShowCursor(SDL_DISABLE);

#endif

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

#if SDL_MAJOR_VERSION == 2
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
		SDL_RenderClear(renderer);
#else
    SDL_FillRect(screen, 0, 0);
#endif

		snprintf(tmp, sizeof(tmp), "Current fps = %.1f", fps);
		SDL_Surface *textSurface = TTF_RenderText_Solid(font, tmp, foregroundColor);

		if(textSurface) {
#if SDL_MAJOR_VERSION == 2
			SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
#endif

			SDL_Rect location;
			location.h = textSurface->h;
			location.w = textSurface->w;
			location.x = position;
			location.y = height / 2 - textSurface->h / 2;
#if SDL_MAJOR_VERSION == 1
      SDL_BlitSurface(textSurface, 0, screen, &location);
#endif
			SDL_FreeSurface(textSurface);

			position += delta;
			if(position >= width - textSurface->w || position <= 0)
				delta *= -1;

#if SDL_MAJOR_VERSION == 2
			if(textTexture) {
				SDL_RenderCopy(renderer, textTexture, 0, &location);
				SDL_DestroyTexture(textTexture);
			}
#endif
		}

#if SDL_MAJOR_VERSION == 2
		SDL_RenderPresent(renderer);
#else
    SDL_Flip(screen);
#endif

		Uint32 end = SDL_GetTicks();
		Uint32 elapsed = end - lastFrame;

		// delay for the remainder of the base rate so we keep a decent frame rate if there's no vsync
		if(vsync == SDL_FALSE && frameRate > elapsed)
			SDL_Delay(frameRate - elapsed);

		if(elapsed > frameRate * 3) {
			struct timeb tp;
			ftime(&tp);
			time_t now;
			struct tm *ti;
			char buf[255];
			time(&now);
			ti = localtime(&now);
			strftime(buf, sizeof(buf), "%I:%M:%S", ti);
			printf("%s.%d: Frame took %ums\n", buf, tp.millitm, elapsed);
		}

		// remember now as the starting point for the next frame
		lastFrame = SDL_GetTicks();

		// update statistics
		elapsed = lastFrame - start;
		fps = 1000.0f / (float)elapsed;
		//printf("Current frame rate is %6.3f fps (%ums)              \r", fps, elapsed);
	}
	printf("\n");

cleanup:
#if SDL_MAJOR_VERSION == 2
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
#else
  SDL_FreeSurface(screen);
#endif
	TTF_Quit();
	SDL_Quit();

	return 0;
}
