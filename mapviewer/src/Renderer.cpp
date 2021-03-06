#include <cmath>
#include <sstream>
#include <queue>

#include <allegro5/allegro.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <allegro5/shader.h>
#include "al_ext.h"

#include "Renderer.h"
#include "ChunkData.h"

#include "Resource/Manager.h"
#include "Resource/AtlasSheet.h"

#include "Level.h"
#include "Map.h"
#include "Vector.h"

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
	
	if(resManager_)
		delete resManager_;
	
	for(auto chunk: chunkData_)
	{
		delete chunk.second;
	}
	
	chunkData_.clear();

	if(dpy_)
		al_destroy_display(dpy_);

	if(al_is_system_installed())
		al_uninstall_system();

	tmr_ = nullptr;
	dpy_ = nullptr;
	queue_ = nullptr;
	prg_ = nullptr;

	resManager_ = nullptr;

	// TODO: delete loaded chunks?
}

void Renderer::setLevel(Level *level)
{
	level_ = level;
	dim0_ = 0;

	for(auto &map: level->maps())
	{
		NBT_Debug("setLevel: dim=%i", map->dimension());
		if(map->dimension() == 0)
		{
			dim0_ = map;
			break;
		}
	}

	if (dim0_ == 0)
	{
		NBT_Debug("No \"dimensions 0\" map found. Defaulting to the first map in the list.");
		dim0_ = level->maps()[0];
	}

	NBT_Debug("spawn %ix%i chunk %ix%i region %ix%i", level->spawnX(), level->spawnZ(), level->spawnX() >> 4, level->spawnZ() >> 4, (level->spawnX()>>4) >> 5, (level->spawnZ()>>4) >> 5);

	camera_pos_ = Vector3D(level->spawnX(), 84, level->spawnZ());
	autoLoadChunks(level->spawnX() >> 4, level->spawnZ() >> 4);
}

Level *Renderer::getLevel()
{
	return level_;
}

bool Renderer::init(Minecraft *mc, const char *argv0)
{
	NBT_Debug("begin");

	al_set_org_name("mctools");
	al_set_app_name("viewer");

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
	ALLEGRO_FONT *fnt = nullptr;

	if(!al_install_keyboard())
		goto init_failed;

   if(!al_install_mouse())
		goto init_failed;

	if(!al_init_primitives_addon())
		goto init_failed;

	if(!al_init_image_addon())
		goto init_failed;

	if(!al_init_font_addon())
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

	dpy = al_create_display(800, 600);

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

	glBindVertexArray(0);

	NBT_Debug("create resource manager");
	resManager_ = new ResourceManager(this);
	if(!resManager_->init(mc, argv0))
	{
		NBT_Debug("failed to init resource manager");
		goto init_failed;
	}

	fnt = al_create_builtin_font();
	if(!fnt)
	{
		NBT_Debug("failed to create builtin font");
		goto init_failed;
	}

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
	fnt_ = fnt;
	grab_mouse_ = false;

	// initial clear display
	// make things look purdy
	al_clear_to_color(al_map_rgb(0,0,0));
   al_flip_display();

	NBT_Debug("end");
	return true;

init_failed:
	delete resManager_;
	resManager_ = nullptr;

	if(fnt)
		al_destroy_font(fnt);

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

	al_hide_mouse_cursor(dpy_);

	al_identity_transform(&camera_transform_);

	float x = -camera_pos_.x, y = -camera_pos_.y, z = -camera_pos_.z;
	//x = -dim0_->spawnX();
	//z = -dim0_->spawnZ();

	al_translate_transform_3d(&camera_transform_, x, y, z);

	al_rotate_transform_3d(&camera_transform_, 0.0, 1.0, 0.0, DEG_TO_RAD(180));

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
	bool doUpdateLookPos = false;
	bool cleared = false;
	
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
			bool changeTranslation = false;
			bool changeRotation = false;

			if(key_state_[ALLEGRO_KEY_W])
			{
				z += translate_diff;
				changeTranslation = true;
			}

			if(key_state_[ALLEGRO_KEY_S])
			{
				z -= translate_diff;
				changeTranslation = true;
			}

			if(key_state_[ALLEGRO_KEY_A])
			{
				x += translate_diff;
				changeTranslation = true;
			}

			if(key_state_[ALLEGRO_KEY_D])
			{
				x -= translate_diff;
				changeTranslation = true;
			}

			if(key_state_[ALLEGRO_KEY_SPACE])
			{
				y -= translate_diff;
				changeTranslation = true;
			}

			if(key_state_[ALLEGRO_KEY_LSHIFT])
			{
				y += translate_diff;
				changeTranslation = true;
			}

			if(key_state_[ALLEGRO_KEY_LEFT])
			{
				ry += rotate_diff;
				changeRotation = true;
			}

			if(key_state_[ALLEGRO_KEY_RIGHT])
			{
				ry -= rotate_diff;
				changeRotation = true;
			}

			if(changeTranslation)
			{
				//camera_pos_.translate(x, y, z);
				al_translate_transform_3d(&camera_transform_, x, y, z);
				doUpdateLookPos = true;
			}

			if(changeRotation)
			{
				al_rotate_transform_3d(&camera_transform_, 0.0, 1.0, 0.0, ry);
				doUpdateLookPos = true;
			}

			if(doUpdateLookPos)
				updateLookPos();

      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			NBT_Debug("display close");
         break;
      }
      else if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			//NBT_Debug("key down");
			//NBT_Debug("pos: %fx%f", -camera_transform_.m[2][0], -camera_transform_.m[2][2]);
			key_state_[ev.keyboard.keycode] = true;

			if (ev.keyboard.keycode == ALLEGRO_KEY_Q)
			{
				break;
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_C)
			{
				NBT_Debug("CLEAR CHUNKS");
				glBindVertexArray(vao_);
				for(auto ch: chunkData_)
				{
					delete ch.second;
				}
				glBindVertexArray(0);
				chunkData_.clear();

				glDeleteBuffers(1, &vao_);
				
				cleared = true;
			}
			else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
			{
				grab_mouse_ = !grab_mouse_;
			}
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_UP)
		{
			//NBT_Debug("pos: %fx%f", -camera_transform_.m[2][0], -camera_transform_.m[2][2]);
			key_state_[ev.keyboard.keycode] = false;
		}
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
		{
				grab_mouse_ = true;
		}
		else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES && grab_mouse_)
		{
			float dx = ev.mouse.dx, dy = ev.mouse.dy;

			if(dy > 0 && dy < 1.5)
				dy = 0.0;

			if(dy < 0 && dy > -1.5)
				dy = 0.0;

			if(dx > 0 && dx < 1.5)
				dy = 0.0;

			if(dx < 0 && dx > -1.5)
				dx = 0.0;

			float ry = dx / al_get_display_width(dpy_), rx = dy / al_get_display_height(dpy_);

			rx_look += rx;
			al_rotate_transform_3d(&camera_transform_, 0.0, 1.0, 0.0, ry);
//			al_rotate_transform_3d(&camera_transform_, 1.0, 0.0, 0.0, rx);

			//cam_.rx += dy / al_get_display_height(dpy_);

			al_set_mouse_xy(dpy_, al_get_display_width(dpy_)/2.0, al_get_display_height(dpy_)/2.0);

			doUpdateLookPos = true;
		}

      if(redraw && al_is_event_queue_empty(queue_))
		{
			if(!loadChunkQueue.empty())
			{
				NBT_Debug("%i chunks to load", loadChunkQueue.size());

				std::pair<int32_t, int32_t> pos = loadChunkQueue.front();
				loadChunkQueue.pop();

				processChunk(pos.first, pos.second);
			}
			else
			{
				if(!cleared)
				{
					//NBT_Debug("pos: %fx%fx%f", camera_pos_.getX(), camera_pos_.getZ(), camera_pos_.getY());
					autoLoadChunks(camera_pos_.getX() / 16.0, camera_pos_.getZ() / 16.0);
				}
			}

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


	NBT_Debug("sizeof GL_FLOAT: %i", sizeof(GLfloat));
}

void Renderer::setupProjection(ALLEGRO_TRANSFORM *m)
{
   int dw = al_get_display_width(dpy_);
   int dh = al_get_display_height(dpy_);
//   al_perspective_transform(m, -180 * dw / dh, -180, 1,
//      180 * dw / dh, 180, 1000);
//	al_perspective_transform(m, -500, -500, 1,
//      500, 500, 10000);

	double zNear = 0.5, zFar = 1000.0, fov = 90.0, aspect = dw / dh;

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

void Renderer::negateTransform(ALLEGRO_TRANSFORM *m)
{
	m->m[0][0] = -m->m[0][0];
	m->m[0][1] = -m->m[0][1];
	m->m[0][2] = -m->m[0][2];

	m->m[1][0] = -m->m[1][0];
	m->m[1][1] = -m->m[1][1];
	m->m[1][2] = -m->m[1][2];

	m->m[2][0] = -m->m[1][0];
	m->m[2][1] = -m->m[1][1];
	m->m[2][2] = -m->m[1][2];
}

void Renderer::draw()
{
	//int dw = al_get_display_width(dpy_);
   //int dh = al_get_display_height(dpy_);

	ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);

	al_compose_transform(&trans, &camera_transform_);
	al_rotate_transform_3d(&trans, 1.0, 0.0, 0.0, rx_look);

	setupProjection(&trans);
	al_identity_transform(&trans);


	al_use_transform(&trans);

	getWorldPos(camera_pos_);

	al_copy_transform(&cur3d_transform_, al_get_current_transform());

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
		ChunkData *cd = it.second;

		ALLEGRO_TRANSFORM ctrans;
		al_identity_transform(&ctrans);
		al_translate_transform_3d(&ctrans, cd->x()*15.0, 0.0, cd->z()*15.0);
		al_use_transform(&ctrans);

		cd->draw(&ctrans);
	}

	glBindVertexArray(0);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//drawSelection();

	resManager_->unsetAtlasUniforms();
}

void Renderer::drawHud()
{
	if(!setShader(SHADER_ALLEGRO))
		NBT_Debug("failed to set allegro shader");

	//al_use_shader(nullptr);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	//int dw = al_get_display_width(dpy_);
   //int dh = al_get_display_height(dpy_);

	/*ALLEGRO_TRANSFORM trans;
	al_identity_transform(&trans);
	al_orthographic_transform(&trans, 0, 0, -1, dw, dh, 1);
	al_set_projection_transform(dpy_, &trans);
	al_identity_transform(&trans);
	al_use_transform(&trans);
	*/

	if(!resManager_->getAtlas()->getSheet(0)->alBitmap())
		NBT_Debug("no sheet bitmap????");

	//al_use_shader(nullptr);section_y
	ALLEGRO_BITMAP *tex = resManager_->getAtlas()->getSheet(0)->alBitmap();
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, al_get_opengl_texture(tex));
	//al_set_shader_sampler("al_tex", tex, 0);
	al_draw_bitmap(tex, 0, 0, 0);

	float x = 0.0, y = 0.0;
	al_transform_coordinates(&camera_transform_, &x, &y);

	Vector3D world_pos;


	al_draw_textf(fnt_, al_map_rgb(0,0,0), 4, al_get_display_height(dpy_)-12, 0, "Pos: x:%.0f, y:%.0f, z:%.0f", camera_pos_.x, camera_pos_.y, camera_pos_.z);

//	al_draw_textf(fnt_, al_map_rgb(0,0,0), 4, al_get_display_height(dpy_)-24, 0, "WP: %.0f %.0f %.0f", world_pos.x, world_pos.y, world_pos.z);
	//al_draw_textf(fnt_, al_map_rgb(0,0,0), 4, al_get_display_height(dpy_)-24, 0, "Mat: x0:%.0f x1:%.0f x2:%.0f x3:%.0f z0:%.0f z1:%.0f z2:%.0f z3:%.0f",
	//				  world.m[0][0], world.m[1][0], world.m[2][0], world.m[3][0],
	//				  world.m[2][2], world.m[2][2], world.m[2][2], world.m[3][2]);
	//al_draw_textf(fnt_, al_map_rgb(0,0,0), 4, al_get_display_height(dpy_)-12, 0, "Pos: x:%.0f y:%.0f z:%.0f", camera_pos_.getX(), camera_pos_.getY(), camera_pos_.getZ());
}

void Renderer::updateLookPos()
{
	ALLEGRO_TRANSFORM look;
	al_copy_transform(&look, &camera_transform_);

	for(int i = 0; i < 4; i++)
	{
		al_translate_transform_3d(&look, 0.0, 0.0, 1.0);

		Vector3D pos;
		unProject(&look, pos);

		int x = (int)floor(pos.x+0.5), z = (int)floor(pos.z+0.5);
		int cx = x / 16, cz = z / 16;
		//int bx = x % 16, bz = z % 16;

		// getChunk will cause regions and chunks to be loaded, but they should already be loaded by now...

		auto it = chunkData_.find(getChunkKey(cx, cz));
		if(it != chunkData_.end() && it->second)
		{
			//RendererChunk *rc = it->second;
			// DO STUFF
		}
	}


}

void Renderer::unProject(ALLEGRO_TRANSFORM *trans, Vector3D &pos)
{
	al_unproject_transform_3d(trans, &(pos.x), &(pos.y), &(pos.z));
}

void Renderer::getWorldPos(Vector3D &pos)
{
	unProject(&camera_transform_, pos);
}

void Renderer::transposeTransform(ALLEGRO_TRANSFORM *t)
{
	ALLEGRO_TRANSFORM copy;
	al_copy_transform(&copy, t);

	t->m[0][0] = copy.m[0][0];
	t->m[0][1] = copy.m[1][0];
	t->m[0][2] = copy.m[2][0];

	t->m[1][0] = copy.m[0][1];
	t->m[1][1] = copy.m[1][1];
	t->m[1][2] = copy.m[2][1];

	t->m[2][0] = copy.m[0][2];
	t->m[2][1] = copy.m[1][2];
	t->m[2][2] = copy.m[2][2];
}

bool Renderer::chunkDataExists(int32_t x, int32_t z)
{
	return chunkData_.count(getChunkKey(x, z)) > 0;
}

void Renderer::processChunk(int x, int z)
{
	NBT_Debug("chunk %ix%i", x, z);

	if(chunkDataExists(x,z))
	{
		NBT_Debug("chunk exists?");
		return;
	}

	Chunk *chunk = dim0_->getChunk(x, z);
	if(!chunk)
	{
		NBT_Debug("failed to get chunk @ %ix%i", x, z);
		return;
	}

	auto cdata = ChunkData::Create(chunk, resManager_);
	if(!cdata)
   {
		NBT_Debug("failed to create chunkdata for chunk @ %ix%i", x, z);
		return;
   }

   if(chunkDataExists(x, z))
	{
		NBT_Debug("WE ALREADY HAVE THIS CHUNK, WTF?", x, z);
		assert(0);
		return;
	}
	
	chunkData_.emplace(getChunkKey(x, z), cdata);
}

void Renderer::autoLoadChunks(int x, int y)
{
	// get the actual platform\almsvc.hdirection we are facing in the x and z axis[s]
	//Vector3D dirVec { camera_transform_.m[2][0], camera_transform_.m[2][2], 0.0 };
	//dirVec.normalize();
	//NBT_Debug("dir: %f,%f", dirVec.x, dirVec.y);

	Vector3D originVec = Vector3D{ (float)x, (float)y, 0.0};//dirVec;
	//originVec += Vector3D{ x, y, 0.0 };     // create vector based on given x,y and current direction
	//originVec = Vector3D{ x, y, 0.0};

	std::unordered_map<Vector3D, bool> checkedMap;

	std::queue<Vector3D> queue;
	queue.push(originVec);

	do
	{
		Vector3D nextchunk = queue.front();
		queue.pop();

		checkedMap.emplace(nextchunk, true);

		auto pos = std::pair<int32_t, int32_t>(nextchunk.x, nextchunk.y);
		if(!chunkDataExists(pos.first, pos.second))
		{
			NBT_Debug("queue %ix%i", (int)nextchunk.x, (int)nextchunk.y);
			loadChunkQueue.push(pos);
		}
		//else {
		//	NBT_Debug("chunk %ix%i !exist", pos.first, pos.second);
		//}

		Vector3D northVec{ nextchunk.x, nextchunk.y+1 };
		if(!checkedMap.count(northVec) && isChunkVisible(originVec, northVec) /*&& !chunkDataExists(northVec.x, northVec.y)*/)
		{
			checkedMap.emplace(northVec, true);
			queue.push(northVec);
		}

		Vector3D westVec{ nextchunk.x+1, nextchunk.y };
		if(!checkedMap.count(westVec) && isChunkVisible(originVec, westVec) /*&& !chunkDataExists(westVec.x, westVec.y)*/)
		{
			checkedMap.emplace(westVec, true);
			queue.push(westVec);
		}

		Vector3D southVec{ nextchunk.x, nextchunk.y-1 };
		if(!checkedMap.count(southVec) && isChunkVisible(originVec, southVec) /*&& !chunkDataExists(southVec.x, southVec.y)*/)
		{
			checkedMap.emplace(southVec, true);
			queue.push(southVec);
		}

		Vector3D eastVec{ nextchunk.x-1, nextchunk.y };
		if(!checkedMap.count(eastVec) && isChunkVisible(originVec, eastVec) /*&& !chunkDataExists(eastVec.x, eastVec.y)*/)
		{
			checkedMap.emplace(eastVec, true);
			queue.push(eastVec);
		}

	} while(queue.size());

}

bool Renderer::isChunkVisible(Vector3D origin, Vector3D pos)
{
	Vector3D dist = pos - origin;

	if(dist.magnitude() <= 4.0)
		return true;

	return false;
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
	const char *slog = nullptr;
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

	slog = al_get_shader_log(prg);
	if(slog)
	{
		NBT_Debug("shader log:\n%s", slog);
	}
	else
	{
		NBT_Debug("no shader log");
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

	if(!al_set_shader_sampler(sstr.str().c_str(), nullptr, sheet->id()))
	{
		NBT_Debug("failed to unset sampler %s", sstr.str().c_str());
	}
}

bool Renderer::setShaderSampler(AtlasSheet *sheet)
{
	std::stringstream sstr;
	sstr << "atlas_sheet_";
	sstr << sheet->id();

	if(!al_set_shader_sampler(sstr.str().c_str(), sheet->alBitmap(), sheet->id()))
	{
		NBT_Debug("failed to set sampler %s", sstr.str().c_str());
		return false;
	}

	return true;
}
