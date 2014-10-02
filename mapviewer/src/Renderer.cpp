#include <cmath>
#include <sstream>

#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_image.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <allegro5/shader.h>

#include "Renderer.h"
#include "ChunkData.h"
#include "Resource/Manager.h"
#include "Resource/AtlasSheet.h"
#include "Level.h"
#include "Map.h"

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
	NBT_Debug("begin");
	
	if(!al_init())
	{
		NBT_Debug("al_init failed???");
		return false;
	}
	
	ALLEGRO_TIMER *tmr = nullptr;
	ALLEGRO_EVENT_QUEUE *queue = nullptr;
	ALLEGRO_DISPLAY *dpy = nullptr;
	ALLEGRO_BITMAP *bmp = nullptr;
	ALLEGRO_TRANSFORM *def_trans = nullptr;
	
	if(!al_install_keyboard())
		goto init_failed;
	
   if(!al_install_mouse())
		goto init_failed;
	
	if(!al_init_primitives_addon())
		goto init_failed;
	
	if(!al_init_image_addon())
		goto init_failed;
	
	tmr = al_create_timer(1.0/60.0);
	if(!tmr)
		goto init_failed;
	
	queue = al_create_event_queue();
	if(!queue)
		goto init_failed;
	
	// do display creation last so a display isn't created and instantly destroyed if any of the
	// preceeding initializations fail.
	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE | ALLEGRO_OPENGL_3_0);
	//al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_REQUIRE);
   //al_set_new_display_option(ALLEGRO_SAMPLES, 4, ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 24, ALLEGRO_REQUIRE);
	
	dpy = al_create_display(1024, 768);
	
	if(!dpy)
	{
		NBT_Debug("display creation failed");
		goto init_failed;
	}
	
	if(!al_get_opengl_extension_list()->ALLEGRO_GL_EXT_framebuffer_object)
	{
		NBT_Debug("FBO GL extension is missing. bail");
		goto init_failed;
	}
	
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);
	
	NBT_Debug("load shaders");
	if(!loadShaders("shaders/default.vtx", "shaders/default.pxl"))
	{
		NBT_Debug("shader init failed");
		goto init_failed;
	}
	
	NBT_Debug("load allegro shaders");
	if(!loadAllegroShaders())
	{
		NBT_Debug("allegro shader init failed");
		goto init_failed;
	}
	
	NBT_Debug("create resource manager");
	resManager_ = new ResourceManager(this);
	
	bmp = al_create_bitmap(100, 100);
	if(!bmp)
	{
		NBT_Debug("failed to create silly bitmap");
		goto init_failed;
	}
	
	al_set_target_bitmap(bmp);
	al_clear_to_color(al_map_rgb(255,255,255));
	al_set_target_backbuffer(dpy);
	
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_display_event_source(dpy));
	al_register_event_source(queue, al_get_timer_event_source(tmr));
	
	def_trans = al_get_projection_transform(dpy);
	al_copy_transform(&al_proj_transform_, def_trans);
	
	al_identity_transform(&camera_transform_);
	
	rx_look = 0.0;
	
	queue_ = queue;
	tmr_ = tmr;
	dpy_ = dpy;
	bmp_ = bmp;
	
	// initial clear display
	// make things look purdy
	al_clear_to_color(al_map_rgb(0,0,0)); 
   al_flip_display();

	NBT_Debug("end");
	return true;
	
init_failed:
	if(dpy)
		al_destroy_display(dpy);
	if(queue)
		al_destroy_event_queue(queue);
	al_uninstall_system();
	NBT_Debug("end");
	return false;
}

void Renderer::run()
{
	NBT_Debug("begin");
	
	al_identity_transform(&camera_transform_);
	al_translate_transform_3d(&camera_transform_, -1, -64, -112);

	memset(key_state_, 0, sizeof(key_state_) * sizeof(key_state_[0]));
	
	al_start_timer(tmr_);
	
	NBT_Debug("run!");
	
	//al_use_shader(nullptr);
	
	/*ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_orthographic_transform(&trans, 0, 0, -1, al_get_display_width(dpy_), al_get_display_height(dpy_), 1);
	al_set_projection_transform(dpy_, &trans);
	al_identity_transform(&trans);
	al_use_transform(&trans);
	
	if(!resManager_->getAtlas()->getSheet(0)->alBitmap())
		NBT_Debug("no sheet bitmap????");
	*/
	//al_draw_bitmap(resManager_->getAtlas()->getSheet(0)->alBitmap(), 0, 0, 0);
	
	//al_flip_display();
	//sleep(10);
	
	bool redraw = false;
	while(1)
	{
		ALLEGRO_EVENT ev;
      al_wait_for_event(queue_, &ev);
 
      if(ev.type == ALLEGRO_EVENT_TIMER)
		{
         redraw = true;
			//cam_.rx = 1.0;
			float x = 0.0, y = 0.0, z = 0.0;
			float translate_diff = 0.3;
			float ry = 0.0;
			float rotate_diff = 0.04;
			
			if(key_state_[ALLEGRO_KEY_W])
				z += translate_diff;
			
			if(key_state_[ALLEGRO_KEY_S])
				z -= translate_diff;
			
			if(key_state_[ALLEGRO_KEY_A])
				x += translate_diff;
			
			if(key_state_[ALLEGRO_KEY_D])
				x -= translate_diff;
			
			if(key_state_[ALLEGRO_KEY_SPACE])
				y -= translate_diff;
			
			if(key_state_[ALLEGRO_KEY_LSHIFT])
				y += translate_diff;
			
			if(key_state_[ALLEGRO_KEY_LEFT])
				ry += rotate_diff;
			
			if(key_state_[ALLEGRO_KEY_RIGHT])
				ry -= rotate_diff;
			
			al_translate_transform_3d(&camera_transform_, x, y, z);
			al_rotate_transform_3d(&camera_transform_, 0.0, 1.0, 0.0, ry);
			
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			NBT_Debug("display close");
         break;
      }
      else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			//NBT_Debug("key down");
			key_state_[ev.keyboard.keycode] = true;
			
			if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				break;
			}
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			key_state_[ev.keyboard.keycode] = false;
		}
		else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES)
		{
			float dx = ev.mouse.dx, dy = ev.mouse.dy;
			float ry = dx / al_get_display_width(dpy_), rx = dy / al_get_display_height(dpy_);
			
			rx_look -= rx;
			al_rotate_transform_3d(&camera_transform_, 0.0, 1.0, 0.0, ry);
//			al_rotate_transform_3d(&camera_transform_, 1.0, 0.0, 0.0, rx);
			
			//cam_.rx += dy / al_get_display_height(dpy_);
			al_set_mouse_xy(dpy_, al_get_display_width(dpy_)/2.0, al_get_display_height(dpy_)/2.0);
		}
 
      if(redraw && al_is_event_queue_empty(queue_))
		{
			ALLEGRO_STATE state;
			al_store_state(&state, ALLEGRO_STATE_ALL);
			al_set_projection_transform(dpy_, &al_proj_transform_);
			
			glClear(GL_DEPTH_BUFFER_BIT);
			
         redraw = false;
			al_clear_to_color(al_map_rgb(255,255,255));
         draw();
			
			al_restore_state(&state);
			al_set_projection_transform(dpy_, &al_proj_transform_);
			
			drawHud();
			
			al_restore_state(&state);
         al_flip_display();
      }
      
      
	}
	
	NBT_Debug("stop timer");
	al_stop_timer(tmr_);
	
	NBT_Debug("end");
}

void Renderer::setupProjection(ALLEGRO_TRANSFORM *m)
{
   int dw = al_get_display_width(dpy_);
   int dh = al_get_display_height(dpy_);
//   al_perspective_transform(m, -180 * dw / dh, -180, 1,
//      180 * dw / dh, 180, 1000);
//	al_perspective_transform(m, -500, -500, 1,
//      500, 500, 10000);
	
	double zNear = 0.5, zFar = 500.0, fov = 40.0, aspect = dw / dh;
	
	double left, right;
	double bottom, top;
	top = tan (fov*ALLEGRO_PI/360.0)*zNear;
	bottom = -top;
	left = aspect*bottom;
	right = aspect*top;
 
	//NBT_Debug("rect: %f %f %f %f", left, top, right, bottom);
	
	al_perspective_transform(m, left, top, zNear,
      right, bottom, zFar);
	
   al_set_projection_transform(dpy_, m);
}

void Renderer::draw()
{
	int dw = al_get_display_width(dpy_);
   int dh = al_get_display_height(dpy_);
	
	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_compose_transform(&trans, &camera_transform_);
	
	setupProjection(&trans);
	al_identity_transform(&trans);
	al_rotate_transform_3d(&trans, 1.0, 0.0, 0.0, rx_look);
	
	
	al_use_transform(&trans);
	
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_ALPHA_TEST);
	
	if(!setShader(SHADER_DEFAULT))
	{
		NBT_Debug("failed to set default shader");
	}
	
	glBindVertexArray(vao_);
		
	resManager_->setAtlasUniforms();
	
	for(auto &it: chunkData_)
	{
		it.second->draw();
	}
	
	glBindVertexArray(0);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	resManager_->unsetAtlasUniforms();
}

void Renderer::drawHud()
{
	if(!setShader(SHADER_ALLEGRO))
		NBT_Debug("failed to set allegro shader");
	
	//al_use_shader(nullptr);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	int dw = al_get_display_width(dpy_);
   int dh = al_get_display_height(dpy_);
	
	/*ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_orthographic_transform(&trans, 0, 0, -1, dw, dh, 1);
	al_set_projection_transform(dpy_, &trans);
	al_identity_transform(&trans);
	al_use_transform(&trans);
	*/
	
	if(!resManager_->getAtlas()->getSheet(0)->alBitmap())
		NBT_Debug("no sheet bitmap????");
	
	//al_use_shader(nullptr);
	ALLEGRO_BITMAP *tex = resManager_->getAtlas()->getSheet(0)->alBitmap();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, al_get_opengl_texture(tex));
	//al_set_shader_sampler("al_tex", tex, 0);
	al_draw_bitmap(tex, 0, 0, 0);
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
	
	ChunkData *cdata = ChunkData::Create(chunk, resManager_);
	if(!cdata)
	{
		NBT_Debug("failed to create chunkdata for chunk @ %ix%i", x, z);
		return;
	}
	
	chunkData_.emplace(getChunkKey(x, z), cdata);
}

bool Renderer::loadAllegroShaders()
{
	ALLEGRO_SHADER *prg = al_create_shader(ALLEGRO_SHADER_GLSL);
	const char *vs = al_get_default_shader_source(ALLEGRO_SHADER_GLSL, ALLEGRO_VERTEX_SHADER);
	const char *ps = al_get_default_shader_source(ALLEGRO_SHADER_GLSL, ALLEGRO_PIXEL_SHADER);
	
	if(!prg)
		goto load_fail;
	
	if(!vs)
		goto load_fail;
	
	if(!ps)
		goto load_fail;
	
	if(!al_attach_shader_source(prg, ALLEGRO_VERTEX_SHADER, vs))
	{
		NBT_Debug("failed to attach vertex shader:\n %s", al_get_shader_log(prg));
		goto load_fail;
	}
	
	if(!al_attach_shader_source(prg, ALLEGRO_PIXEL_SHADER, ps))
	{
		NBT_Debug("failed to attach pixel shader:\n %s", al_get_shader_log(prg));
		goto load_fail;
	}
	
	if(!al_build_shader(prg))
	{
		NBT_Debug("failed to build shader:\n %s", al_get_shader_log(prg));
		goto load_fail;
	}
	
	al_prg_ = prg;
	
	return true;
	
load_fail:
	if(prg)
		al_destroy_shader(prg);
	
	al_prg_ = nullptr;
	
	return false;
}

bool Renderer::loadShaders(const char *vertex_file_path, const char *fragment_file_path)
{
	ALLEGRO_SHADER *prg = al_create_shader(ALLEGRO_SHADER_GLSL);
	if(!prg)
		goto load_fail;
	
	if(!al_attach_shader_source_file(prg, ALLEGRO_VERTEX_SHADER, vertex_file_path))
	{
		NBT_Debug("failed to attach vertex shader:\n %s", al_get_shader_log(prg));
		goto load_fail;
	}
	
	if(!al_attach_shader_source_file(prg, ALLEGRO_PIXEL_SHADER, fragment_file_path))
	{
		NBT_Debug("failed to attach pixel shader:\n %s", al_get_shader_log(prg));
		goto load_fail;
	}
	
	if(!al_build_shader(prg))
	{
		NBT_Debug("failed to build shader:\n %s", al_get_shader_log(prg));
		goto load_fail;
	}
	
	/*if(!al_use_shader(prg))
	{
		NBT_Debug("failed to use shader");
		goto load_fail;
	}*/
	
	prg_ = prg;
	
	return true;
	
load_fail:
	if(prg)
		al_destroy_shader(prg);
	
	return false;
}

bool Renderer::setShader(Renderer::ShaderType type)
{
	switch(type)
	{
		case SHADER_DEFAULT:
			return al_use_shader(prg_);
		
		case SHADER_ALLEGRO:
			return al_use_shader(al_prg_);
	}
	
	return false;
}

void Renderer::unsetShaderSampler(AtlasSheet* sheet)
{
	std::stringstream sstr;
	sstr << "atlas_sheet_";
	sstr << sheet->id();
	
	if(!al_set_shader_sampler(sstr.str().c_str(), nullptr, sheet->id()+1))
	{
		NBT_Debug("failed to unset sampler %s", sstr.str().c_str());
	}
}

bool Renderer::setShaderSampler(AtlasSheet *sheet)
{
	std::stringstream sstr;
	sstr << "atlas_sheet_";
	sstr << sheet->id();
	
	if(!al_set_shader_sampler(sstr.str().c_str(), sheet->alBitmap(), sheet->id()+1))
	{
		NBT_Debug("failed to set sampler %s", sstr.str().c_str());
		return false;
	}

	return true;
}