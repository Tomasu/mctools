#include "Renderer.h"
#include <ChunkData.h>
#include "Level.h"
#include "Map.h"
#include <GL/gl.h>
#include <cmath>

#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/shader.h>

#include "NBT_Debug.h"

Renderer::Renderer() : level_(nullptr), queue_(nullptr), tmr_(nullptr), dpy_(nullptr)
{

}

Renderer::~Renderer()
{
	NBT_Debug("dtor");
	uninit();
}

void Renderer::uninit()
{
	if(queue_)
		al_destroy_event_queue(queue_);
		
	if(tmr_)
		al_destroy_timer(tmr_);
	
	if(prg_)
	{
		al_use_shader(nullptr);
		al_destroy_shader(prg_);
	}
	
	if(dpy_)
		al_destroy_display(dpy_);
	
	if(al_is_system_installed())
		al_uninstall_system();
	
	tmr_ = nullptr;
	dpy_ = nullptr;
	queue_ = nullptr;
	prg_ = nullptr;
}

void Renderer::setLevel(Level *level)
{
	level_ = level;
	
	for(auto &map: level->maps())
	{
		if(map->dimension() == 0)
		{
			dim0_ = map;
			break;
		}
	}
	
	NBT_Debug("spawn %ix%i chunk %ix%i region %ix%i", dim0_->spawnX(), dim0_->spawnZ(), dim0_->spawnX() >> 4, dim0_->spawnZ() >> 4, (dim0_->spawnX()>>4) >> 5, (dim0_->spawnZ()>>4) >> 5);
	
	processChunk(dim0_->spawnX() >> 4, dim0_->spawnZ() >> 4);
}

Level *Renderer::getLevel()
{
	return level_;
}

bool Renderer::init()
{
	if(!al_init())
		return false;
	
	ALLEGRO_TIMER *tmr = nullptr;
	ALLEGRO_EVENT_QUEUE *queue = nullptr;
	ALLEGRO_DISPLAY *dpy = nullptr;
	
	if(!al_install_keyboard())
		goto init_failed;
	
   if(!al_install_mouse())
		goto init_failed;
	
	tmr = al_create_timer(1.0/60.0);
	if(!tmr)
		goto init_failed;
	
	queue = al_create_event_queue();
	if(!queue)
		goto init_failed;
	
	// do display creation last so a display isn't created and instantly destroyed if any of the
	// preceeding initializations fail.
	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);
	dpy = al_create_display(800, 600);
	
	if(!dpy)
		goto init_failed;
	
	if(!al_get_opengl_extension_list()->ALLEGRO_GL_EXT_framebuffer_object)
	{
		NBT_Debug("FBO GL extension is missing. bail");
		goto init_failed;
	}
	
	if(!loadShaders("shaders/default.vtx", "shaders/default.pxl"))
	{
		NBT_Debug("shader init failed");
		goto init_failed;
	}
	
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(dpy));
	al_register_event_source(queue, al_get_timer_event_source(tmr));
	
	queue_ = queue;
	tmr_ = tmr;
	dpy_ = dpy;

	// initial clear display
	// make things look purdy
	al_clear_to_color(al_map_rgb(0,0,0)); 
   al_flip_display();
	
	return true;
	
init_failed:
	if(dpy)
		al_destroy_display(dpy);
	if(queue)
		al_destroy_event_queue(queue);
	al_uninstall_system();
	return false;
}

void Renderer::run()
{
	al_start_timer(tmr_);
	
	NBT_Debug("run!");
	
	bool redraw = false;
	while(1)
	{
		ALLEGRO_EVENT ev;
      al_wait_for_event(queue_, &ev);
 
      if(ev.type == ALLEGRO_EVENT_TIMER)
		{
         redraw = true;
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			NBT_Debug("display close");
         break;
      }
      else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			NBT_Debug("key down");
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				break;
			}
		}

 
      if(redraw && al_is_event_queue_empty(queue_)) {
         redraw = false;
         draw();
         al_flip_display();
      }
	}
	
	NBT_Debug("stop timer");
	al_stop_timer(tmr_);
	
	NBT_Debug("done");
}

void Renderer::setupProjection(ALLEGRO_TRANSFORM *m)
{
   int dw = al_get_display_width(dpy_);
   int dh = al_get_display_height(dpy_);
   al_perspective_transform(m, -180 * dw / dh, -180, 180,
      180 * dw / dh, 180, 3000);
   al_set_projection_transform(dpy_, m);
}

void Renderer::draw()
{
/*	GLdouble zNear = 1.0, zFar = 1000.0;
	GLdouble fovY = 45.0;
	GLdouble fW, fH;
	GLdouble aspect = (GLdouble)al_get_display_height(dpy_) / (GLdouble)al_get_display_width(dpy_);
	fH = tan( fovY / 360 * M_PI ) * zNear;
	fW = fH * aspect;

	glFrustum( -fW, fW, -fH, fH, zNear, zFar );
*/
	//glFrustum(float left, float right, float bottom, float top, float near, float far);
	
	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_translate_transform_3d(&trans, 0, 0, 20);
	setupProjection(&trans);
		
	for(auto &it: chunkData_)
	{
		it.second->draw();
	}
}

bool Renderer::chunkDataExists(int32_t x, int32_t z)
{
	return chunkData_.count(getChunkKey(x, z)) > 0;
}

void Renderer::processChunk(int x, int z)
{
	Chunk *chunk = dim0_->getChunk(x, z);
	if(!chunk)
	{
		NBT_Debug("failed to get chunk @ %ix%i", x, z);
		return;
	}
	
	ChunkData *cdata = ChunkData::Create(chunk);
	if(!cdata)
	{
		NBT_Debug("failed to create chunkdata for chunk @ %ix%i", x, z);
		return;
	}
	
	chunkData_.emplace(getChunkKey(x, z), cdata);
}

bool Renderer::loadShaders(const char *vertex_file_path, const char *fragment_file_path)
{
	ALLEGRO_SHADER *prg = al_create_shader(ALLEGRO_SHADER_GLSL);
	if(!prg)
		goto load_fail;
	
	if(!al_attach_shader_source_file(prg, ALLEGRO_VERTEX_SHADER, vertex_file_path))
	{
		NBT_Debug("failed to attach vertex shader");
		goto load_fail;
	}
	
	if(!al_attach_shader_source_file(prg, ALLEGRO_PIXEL_SHADER, fragment_file_path))
	{
		NBT_Debug("failed to attach pixel shader");
		goto load_fail;
	}
	
	if(!al_build_shader(prg))
	{
		NBT_Debug("failed to build shader");
		goto load_fail;
	}
	
	if(!al_use_shader(prg))
	{
		NBT_Debug("failed to use shader");
		goto load_fail;
	}
	
	prg_ = prg;
	
	return true;
	
load_fail:
	if(prg)
		al_destroy_shader(prg);
	
	return false;
}
