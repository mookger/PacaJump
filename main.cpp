#include <arsa.h>

#define STATE_IDEL	0
#define STATE_INDEX 1
#define STATE_STATE 2
#define STATE_01	3
#define STATE_02	4
#define STATE_03	5
#define STATE_04	6
#define STATE_05	7
#define STATE_06	8
#define STATE_07	9
#define STATE_08	10
#define STATE_09	11
#define STATE_ABOUT	12
int state = STATE_INDEX;

#define CHAR_IDLE		0
#define CHAR_RUN_RIGHT	1
#define CHAR_RUN_LEFT	2
#define CHAR_JUMP_UP	3
#define CHAR_JUMP_DOWN	4

int ku_state = CHAR_IDLE; // run state
int ku_jump_state = CHAR_IDLE; // jump state

int ku_group = -1; // mu group animation

// jump parameters
float ku_jump_speed = 10;
float ku_jump_max = 150;
float ku_jump_count = 0;

irr::core::vector3df ku_pos; // mu position
irr::core::dimension2du ku_size; // mu size in pixel

int score = 0;							//ไว้เช็ค score
int chk_state = 0;						//เช็คด่าน
int st = 0;								//ไว้เช็คให้ข้อความ score ปรากฏ
int count = 0;
int finish = 0;
irr::gui::IGUIFont* font = 0;

int start_pos;

void init()
{
	g_psd->load("source/menu.psd");
	g_psd->loadFont("source/LayijiMahaniyomV1041.ttf", 45);
	font = arsa_CreateFont(g_device, "source/LayijiMahaniyomV1041.ttf", 190);	//สร้างฟ้อนสำหรับ WIN
	g_snd->Load("source/sound/bgsound-main.mp3"); // cache snd
	g_snd->Play("source/sound/bgsound-main.mp3", 0, true); // play bg snd
	
	// get start pos
	int start_pos = g_psd->getPsdId("start_pos");
	g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);

	ku_group = g_psd->getGroupId("g_ku");
	g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);

	ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
	
}

// check user tap left button
bool update_StateLeft()
{
	if (g_input->getTouchState(0) == irr::EKS_RELEASED)
	{
		int x = g_input->getTouchX(0);
		int y = g_input->getTouchY(0);

		int left = g_psd->getPsdIdByPrefix("btn_back");
		if (left != -1)
		{
			if (g_psd->hit(x, y, (irr::u32)left))
			{
				ku_state = CHAR_RUN_LEFT;
				g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("ku_run"), true);
				g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("ku_run"), true);
				g_psd->setGroupDrawFlags(ku_group, irr::arsa::EDT_HFLIP | irr::arsa::EDT_CENTER | irr::arsa::EDT_BOTTOM);
				return true;
			}
		}
	}
	return false;
}

// check user tap right button
bool update_StateRight()
{
	if (g_input->getTouchState(0) == irr::EKS_RELEASED)
	{
		int x = g_input->getTouchX(0);
		int y = g_input->getTouchY(0);
		int right = g_psd->getPsdIdByPrefix("btn_forward");
		if (right != -1)
		{
			if (g_psd->hit(x, y, (irr::u32)right))
			{
				ku_state = CHAR_RUN_RIGHT;
				g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("ku_run"), true);
				g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("ku_run"), true);
				g_psd->setGroupDrawFlags(ku_group, irr::arsa::EDT_CENTER | irr::arsa::EDT_BOTTOM);
				return true;
			}
		}
	}

	return false;
}

// update run state
void update_Run()
{
	irr::f32 speed = 7;
	switch (ku_state)
	{
	case CHAR_IDLE:
	{
		update_StateLeft();
		update_StateRight();
	}
	break; // idle

	case CHAR_RUN_LEFT:
	{
		// exit state
		irr::core::dimension2du size = ku_size / 2;
		if (ku_pos.X <= size.Width)
		{
			ku_state = CHAR_IDLE;
			if (ku_jump_state == CHAR_IDLE)
			{
				g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("ku"), true);
				g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("ku"), true);
			}
			break;
		}

		// change state
		update_StateRight();

		// move scene
		irr::core::vector3df offset = g_psd->getOffset();
		offset.X -= speed;
		g_psd->setOffset(offset);

		// move char
		ku_pos.X -= speed;
	}
	break; // left

	case CHAR_RUN_RIGHT:
	{
		// exit state
		irr::core::dimension2du size = ku_size / 2;
		irr::core::aabbox3df scene_max = g_psd->getMaxOffset();
		if (ku_pos.X >= scene_max.MaxEdge.X - size.Width)
		{
			ku_state = CHAR_IDLE;
			if (ku_jump_state == CHAR_IDLE)
			{
				g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("ku"), true);
				g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("ku"), true);
			}
			break;
		}

		// change state
		update_StateLeft();

		irr::core::dimension2du screen = g_driver->getScreenSize();

		// move scene
		irr::core::vector3df offset = g_psd->getOffset();
		offset.X += speed;
		g_psd->setOffset(offset);

		// move char
		ku_pos.X += speed;
	}
	break; // right
	} // switch
}
void update_Jump()
{
	// move scene
	irr::core::vector3df offset = g_psd->getOffset();

	switch (ku_jump_state)
	{
	case CHAR_IDLE:
	{
		if (g_input->getTouchState(0) == irr::EKS_PRESSED)
		{
			int x = g_input->getTouchX(0);
			int y = g_input->getTouchY(0);

			int bj = g_psd->getPsdIdByPrefix("btn_jump");
			if (bj != -1)
			{
				if (g_psd->hit(x, y, (irr::u32)bj))
				{
					ku_jump_state = CHAR_JUMP_UP;
					ku_jump_count = 0;
					g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("ku_jump"), true);
					g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("ku_jump"), true);
				}
			}
		}
	}
	break;
	case CHAR_JUMP_UP:
	{
		irr::f32 speed = ku_jump_speed;

		// exit state
		if (ku_jump_count >= ku_jump_max)
		{
			ku_jump_state = CHAR_JUMP_DOWN;
			break;
		}

		ku_jump_count += speed;

		// move char
		ku_pos.Y -= speed;

		offset.Y -= speed;
	}
	break;
	case CHAR_JUMP_DOWN:
	{
		irr::f32 speed = ku_jump_speed;

		// exit state
		if (ku_jump_count <= 0)
		{
			ku_jump_count = 0;
			ku_jump_state = CHAR_IDLE;

			irr::core::stringc psd_name;
			if (ku_state == CHAR_IDLE)
				psd_name = "ku";
			else
				psd_name = "ku_run";
			int psd_id = g_psd->getPsdId(psd_name);
			g_psd->setGroupVisibleOne(ku_group, psd_id, true);
			g_psd->setGroupPlayOne(ku_group, psd_id, true);
			break;
		}

		ku_jump_count -= speed;

		// move char
		ku_pos.Y += speed;
		offset.Y += speed;
	}
	break;
	} // switch (mu_state)

	g_psd->setOffset(offset);
}


void index()
{
	// move scene
	irr::core::vector3df offset = g_psd->getOffset();
	g_psd->setOffset(offset);

	//หน้าเมนู
	if (g_input->getTouchState(0) == irr::EKS_RELEASED)		//เมื่อกด tap และจะเช็คว่า tap ที่ตำแหน่งอะไร 
	{
		int x = g_input->getTouchX(0);
		int y = g_input->getTouchY(0);

		if (g_psd->hit(x, y, irr::core::stringc("play")))	//ถ้ากดปุ่ม Start
		{
			state = STATE_STATE;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-aboutme")))	//ถ้ากดปุ่ม Start
		{
			state = STATE_ABOUT;
		}
		if (g_psd->hit(x, y, irr::core::stringc("exit")))	//ถ้ากดปุ่ม Start
		{
			exit(EXIT_SUCCESS);
		}
	}

}

void about()
{
	// move scene
	irr::core::vector3df offset = g_psd->getOffset();
	g_psd->setOffset(offset);

	//หน้าเมนู
	if (g_input->getTouchState(0) == irr::EKS_RELEASED)		//เมื่อกด tap และจะเช็คว่า tap ที่ตำแหน่งอะไร 
	{
		int x = g_input->getTouchX(0);
		int y = g_input->getTouchY(0);

		if (g_psd->hit(x, y, irr::core::stringc("backtomenu")))	//ถ้ากดปุ่ม Start
		{
			state = STATE_INDEX;
		}
	}
}

void state_menu()
{
	if (finish >= 1)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state02"), true);
	if (finish >= 2)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state03"), true);
	if (finish >= 3)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state04"), true);
	if (finish >= 4)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state05"), true);
	if (finish >= 5)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state06"), true);
	if (finish >= 6)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state07"), true);
	if (finish >= 7)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state08"), true);
	if (finish >= 8)
		g_psd->setPsdVisible(g_psd->getPsdId("btn-state09"), true);

	if (g_input->getTouchState(0) == irr::EKS_RELEASED)		//เมื่อกด tap และจะเช็คว่า tap ที่ตำแหน่งอะไร 			
	{
		int x = g_input->getTouchX(0);
		int y = g_input->getTouchY(0);

		if (g_psd->hit(x, y, irr::core::stringc("btn-state01")))
		{
			state = STATE_01;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state02")))
		{
			state = STATE_02;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state03")))
		{
			state = STATE_03;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state04")))
		{
			state = STATE_04;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state05")))
		{
			state = STATE_05;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state06")))
		{
			state = STATE_06;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state07")))
		{
			state = STATE_07;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state08")))
		{
			state = STATE_08;
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-state09")))
		{
			state = STATE_09;
		}
		if (g_psd->hit(x, y, irr::core::stringc("reset")))
		{
			state = STATE_INDEX;
		}
	}
}

void game01()
{
	//ปุ่ม
	if (g_input->getTouchState(0) == irr::EKS_RELEASED)		//เมื่อกด tap และจะเช็คว่า tap ที่ตำแหน่งอะไร 			
	{
		int x = g_input->getTouchX(0);
		int y = g_input->getTouchY(0);

		if (g_psd->hit(x, y, irr::core::stringc("btn_reset hud")))
		{
			state = STATE_STATE;
		}

		if (g_psd->hit(x, y, irr::core::stringc("btn_play hud")))
		{
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn_play"), false);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("quiz"), false);
		}
		if (g_psd->hit(x, y, irr::core::stringc("btn-menu hud")))
		{
			state = STATE_STATE;
		}
	}

	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");
	irr::s32 ku_idle = g_psd->getPsdId("ku_run");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//summer
	irr::s32 summer_s = g_psd->getPsdIdByPrefix("summer_s");
	irr::s32 summer_u = g_psd->getPsdIdByPrefix("summer_u");
	irr::s32 summer_m1 = g_psd->getPsdIdByPrefix("summer_m1");
	irr::s32 summer_m2 = g_psd->getPsdIdByPrefix("summer_m2");
	irr::s32 summer_e = g_psd->getPsdIdByPrefix("summer_e");
	irr::s32 summer_r = g_psd->getPsdIdByPrefix("summer_r");
	/*
	if (ku_idle != -1)
	{
		irr::s32 id1 = g_psd->hit(ku_idle);
		// scope
		if (id1 != -1 && g_psd->getPsdName(id1).find("scope") != -1)
			ku_state = CHAR_IDLE;
	}	
	*/

	if (ku != -1)
	{

		st++;

		//SUMMER
		irr::s32 id = g_psd->hit(ku);

		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("s_s") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(summer_s, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_u") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(summer_u, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_m1") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(summer_m1, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_m2") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(summer_m2, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_e") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(summer_e, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_r") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(summer_r, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}


		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("s_b1") != -1)	//ถ้าเก็บตัว a -> ให้ตัว a หายไป หัวใจลด	
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-01hit.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_b2") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-01hit.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_b3") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-01hit.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_c1") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-01hit.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("s_c2") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-01hit.mp3");
			count++;
		}


		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}

		//เพิ่มพลัง
		if (id != -1 && g_psd->getPsdName(id).find("heartplus1") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_heart"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_heart"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			if (count > 0 && count < 5)
			{
				if (count == 1)
					g_psd->setPsdVisible(heart4, true);
				if (count == 2)
					g_psd->setPsdVisible(heart3, true);
				if (count == 3)
					g_psd->setPsdVisible(heart2, true);
				if (count == 4)
					g_psd->setPsdVisible(heart1, true);

				count--;
			}
		}
		if (id != -1 && g_psd->getPsdName(id).find("heartplus2") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_heart"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_heart"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			if (count > 0 && count < 5)
			{
				if (count == 1)
					g_psd->setPsdVisible(heart4, true);
				if (count == 2)
					g_psd->setPsdVisible(heart3, true);
				if (count == 3)
					g_psd->setPsdVisible(heart2, true);
				if (count == 4)
					g_psd->setPsdVisible(heart1, true);

				count--;
			}
		}

	}
}

void game02()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//summer
	irr::s32 ghost_g = g_psd->getPsdIdByPrefix("ghost-g");
	irr::s32 ghost_h = g_psd->getPsdIdByPrefix("ghost-h");
	irr::s32 ghost_o = g_psd->getPsdIdByPrefix("ghost-o");
	irr::s32 ghost_s = g_psd->getPsdIdByPrefix("ghost-s");
	irr::s32 ghost_t = g_psd->getPsdIdByPrefix("ghost-t");

	if (ku != -1)
	{
		
		st++;
		//GHOST
		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("g-g") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(ghost_g, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-h") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(ghost_h, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-o") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(ghost_o, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-s") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(ghost_s, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-t") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(ghost_t, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("g-c") != -1)	//ถ้าเก็บตัว a -> ให้ตัว a หายไป หัวใจลด	
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-02ghost.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-m") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-02ghost.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-a") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-02ghost.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-b") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-02ghost.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("g-i") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-02ghost.mp3");
			count++;
		}


		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game03()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//forest
	irr::s32 forest_f = g_psd->getPsdIdByPrefix("forest_f");
	irr::s32 forest_o = g_psd->getPsdIdByPrefix("forest_o");
	irr::s32 forest_r = g_psd->getPsdIdByPrefix("forest_r");
	irr::s32 forest_e = g_psd->getPsdIdByPrefix("forest_e");
	irr::s32 forest_s = g_psd->getPsdIdByPrefix("forest_s");
	irr::s32 forest_t = g_psd->getPsdIdByPrefix("forest_t");

	if (ku != -1)
	{

		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("f_f") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(forest_f, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_o") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(forest_o, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_r") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(forest_r, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_e") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(forest_e, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_s") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(forest_s, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_t") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(forest_t, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("f_h") != -1)	//ถ้าเก็บตัว a -> ให้ตัว a หายไป หัวใจลด	
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-03weed.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_n") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-03weed.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_b") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-03weed.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_i") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-03weed.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("f_a") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-03weed.mp3");
			count++;
		}


		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game04()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//forest
	irr::s32 frozen_f = g_psd->getPsdIdByPrefix("frozen_f");
	irr::s32 frozen_r = g_psd->getPsdIdByPrefix("frozen_r");
	irr::s32 frozen_o = g_psd->getPsdIdByPrefix("frozen_o");
	irr::s32 frozen_z = g_psd->getPsdIdByPrefix("frozen_z");
	irr::s32 frozen_e = g_psd->getPsdIdByPrefix("frozen_e");
	irr::s32 frozen_n = g_psd->getPsdIdByPrefix("frozen_n");

	if (ku != -1)
	{

		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("fr_f") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(frozen_f, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_r") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(frozen_r, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_o") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(frozen_o, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_z") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(frozen_z, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_e") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(frozen_e, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_n") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(frozen_n, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("fr_g") != -1)	//ถ้าเก็บตัว a -> ให้ตัว a หายไป หัวใจลด	
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_c") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_i") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_d1") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_d2") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_a") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("fr_s") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-04ice.mp3");
			count++;
		}

		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game05()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//forest
	irr::s32 lotus_l = g_psd->getPsdIdByPrefix("lotus_l");
	irr::s32 lotus_o = g_psd->getPsdIdByPrefix("lotus_o");
	irr::s32 lotus_t = g_psd->getPsdIdByPrefix("lotus_t");
	irr::s32 lotus_u = g_psd->getPsdIdByPrefix("lotus_u");
	irr::s32 lotus_s = g_psd->getPsdIdByPrefix("lotus_s");

	if (ku != -1)
	{

		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("l_l") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(lotus_l, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_o") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(lotus_o, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_t") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(lotus_t, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_u") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(lotus_u, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_s") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(lotus_s, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("l_b") != -1)	//ถ้าเก็บตัว a -> ให้ตัว a หายไป หัวใจลด	
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_y") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_c") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_i") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_h") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_a") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("l_v") != -1)	//ถ้าเก็บตัว b -> ให้ b หายไป 
		{
			g_psd->setPsdVisible(id, false);
			g_snd->Play("source/sound/effect-05lotus.mp3");
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			count++;
		}


		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game06()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//forest
	irr::s32 spirit_s = g_psd->getPsdIdByPrefix("spirit_s");
	irr::s32 spirit_p = g_psd->getPsdIdByPrefix("spirit_p");
	irr::s32 spirit_i1 = g_psd->getPsdIdByPrefix("spirit_i1");
	irr::s32 spirit_r = g_psd->getPsdIdByPrefix("spirit_r");
	irr::s32 spirit_i2 = g_psd->getPsdIdByPrefix("spirit_i2");
	irr::s32 spirit_t = g_psd->getPsdIdByPrefix("spirit_t");

	if (ku != -1)
	{
		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("sp_s") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(spirit_s, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_p") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(spirit_p, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_i1") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(spirit_i1, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_r") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(spirit_r, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_i2") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(spirit_i2, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_t") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(spirit_t, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("sp_k") != -1)	
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_a1") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_a3") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_a2") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_e") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_d") != -1) 
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_w1") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sp_w2") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-06bat.mp3");
			count++;
		}

		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game07()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//crowd
	irr::s32 crowd_c = g_psd->getPsdIdByPrefix("crowd_c");
	irr::s32 crowd_r = g_psd->getPsdIdByPrefix("crowd_r");
	irr::s32 crowd_o = g_psd->getPsdIdByPrefix("crowd_o");
	irr::s32 crowd_w = g_psd->getPsdIdByPrefix("crowd_w");
	irr::s32 crowd_d = g_psd->getPsdIdByPrefix("crowd_d");

	if (ku != -1)
	{
		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("c_c") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(crowd_c, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_r") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(crowd_r, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_o") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(crowd_o, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_w") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(crowd_w, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_d") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(crowd_d, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("c_n1") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-07eletric.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_n2") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-07eletric.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_t") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-07eletric.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_i") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-07eletric.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_l") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-07eletric.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("c_b") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-07eletric.mp3");
			count++;
		}

		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game08()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//crowd
	irr::s32 sakura_s = g_psd->getPsdIdByPrefix("sakura_s");
	irr::s32 sakura_a1 = g_psd->getPsdIdByPrefix("sakura_a1");
	irr::s32 sakura_k = g_psd->getPsdIdByPrefix("sakura_k");
	irr::s32 sakura_u = g_psd->getPsdIdByPrefix("sakura_u");
	irr::s32 sakura_r = g_psd->getPsdIdByPrefix("sakura_r");
	irr::s32 sakura_a2 = g_psd->getPsdIdByPrefix("sakura_a2");

	if (ku != -1)
	{
		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("sa_s") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(sakura_s, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_a1") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(sakura_a1, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_k") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(sakura_k, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_u") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(sakura_u, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_r") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(sakura_r, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_a2") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(sakura_a2, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("sa_n") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_g") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_y") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_o") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_i") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_t") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_p") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("sa_m") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-08sword.mp3");
			count++;
		}

		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void game09()
{
	//GAME
	irr::s32 ku = g_psd->getPsdId("ku_jump");

	//heart
	irr::s32 heart = g_psd->getPsdIdByPrefix("heart");
	irr::s32 heart0 = g_psd->getPsdIdByPrefix("heart0");
	irr::s32 heart1 = g_psd->getPsdIdByPrefix("heart1");
	irr::s32 heart2 = g_psd->getPsdIdByPrefix("heart2");
	irr::s32 heart3 = g_psd->getPsdIdByPrefix("heart3");
	irr::s32 heart4 = g_psd->getPsdIdByPrefix("heart4");

	//crowd
	irr::s32 cowboy_c = g_psd->getPsdIdByPrefix("cowboy_c");
	irr::s32 cowboy_o1 = g_psd->getPsdIdByPrefix("cowboy_o1");
	irr::s32 cowboy_w = g_psd->getPsdIdByPrefix("cowboy_w");
	irr::s32 cowboy_b = g_psd->getPsdIdByPrefix("cowboy_b");
	irr::s32 cowboy_o2 = g_psd->getPsdIdByPrefix("cowboy_o2");
	irr::s32 cowboy_y = g_psd->getPsdIdByPrefix("cowboy_y");

	if (ku != -1)
	{
		st++;

		irr::s32 id = g_psd->hit(ku);
		//ถูกตัว
		if (id != -1 && g_psd->getPsdName(id).find("co_c") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(cowboy_c, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_o1") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(cowboy_o1, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_w") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(cowboy_w, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_b") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(cowboy_b, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_o2") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(cowboy_o2, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_y") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setPsdVisible(cowboy_y, true);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_true"), true);
			g_snd->Play("source/sound/effect-true.mp3");
			score++;
		}

		//ผิดตัว
		if (id != -1 && g_psd->getPsdName(id).find("co_k") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_l") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_q") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_p") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_r") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_a") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}
		if (id != -1 && g_psd->getPsdName(id).find("co_f") != -1)
		{
			g_psd->setPsdVisible(id, false);
			g_psd->setGroupVisibleOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_psd->setGroupPlayOne(ku_group, g_psd->getPsdId("fx_false"), true);
			g_snd->Play("source/sound/effect-09wind.mp3");
			count++;
		}

		//heart
		if (count == 1)
		{
			g_psd->setPsdVisible(heart4, false);
		}
		if (count == 2)
		{
			g_psd->setPsdVisible(heart3, false);
		}
		if (count == 3)
		{
			g_psd->setPsdVisible(heart2, false);
		}
		if (count == 4)
		{
			g_psd->setPsdVisible(heart1, false);
		}
		if (count == 5)
		{
			g_psd->setPsdVisible(heart0, false);
		}
	}
}

void update()
{
	index();
	state_menu();
	about();
	game01();
	game02();
	game03();
	game04();
	game05();
	game06();
	game07();
	game08();
	game09();

	switch (state) {
		case STATE_INDEX:
		{
			g_psd->load("source/menu.psd");	
			g_psd->setOffset(irr::core::vector3df(0, 0, 0));	//**************
			state = STATE_IDEL;
			ku_group = -1;
		}
		break;

		case STATE_ABOUT:
		{
			g_psd->load("source/aboutme.psd");
			g_psd->setOffset(irr::core::vector3df(0, 0, 0));	//**************
			state = STATE_IDEL;
			ku_group = -1;
		}
		break;


		case STATE_STATE:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-main.mp3");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 0;
			g_psd->load("source/state01.psd");
			state = STATE_IDEL;
			ku_group = -1;
		}		
		break;

		case STATE_01:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state01.mp3");
			g_psd->load("source/game01.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 1;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_02:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state02.mp3");
			g_psd->load("source/game02.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 2;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_03:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state03.mp3");
			g_psd->load("source/game03.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 3;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_04:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state04.mp3");
			g_psd->load("source/game04.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 4;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_05:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state05.mp3");
			g_psd->load("source/game05.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 5;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_06:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state06.mp3");
			g_psd->load("source/game06.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 6;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_07:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state07.mp3");
			g_psd->load("source/game07.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 7;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_08:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state08.mp3");
			g_psd->load("source/game08.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 8;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;

		case STATE_09:
		{
			g_snd->StopAllWav();
			g_snd->Play("source/sound/bgsound-state09.mp3");
			g_psd->load("source/game09.psd");
			score = 0;
			count = 0;
			st = 0;
			chk_state = 9;

			// get start pos
			int start_pos = g_psd->getPsdId("start_pos");
			g_psd->getPsdPos(start_pos, ku_pos.X, ku_pos.Y);
			ku_group = g_psd->getGroupId("g_ku");
			g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
			ku_size = g_psd->getPsdSize(g_psd->getPsdId("ku_run"));
			state = STATE_IDEL;
		}
		break;
	}

	update_Run();
	update_Jump();

	// final set mu pos
	if (ku_group != -1)
		g_psd->setGroupPos(ku_group, ku_pos.X, ku_pos.Y);
}

void hud()
{
	/*
	irr::core::stringw fi;
	fi += finish;
	g_psd->drawFont(23, 20, fi, irr::arsa::EDT_DEFAULT, irr::video::SColor(255, 255, 255, 255));
	*/

	if (st>1)
	{
		irr::core::stringw str = "Score: ";
		str += score;
		g_psd->drawFont(23, 65, str, irr::arsa::EDT_DEFAULT, irr::video::SColor(255, 255, 255, 255));

		if (score == 6 && chk_state == 1)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 98, 51, 20), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next1"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 1;
			ku_state = CHAR_IDLE;

		}
		else if (score == 5 && chk_state == 2)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 255, 255, 255), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next2"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 2;
			ku_state = CHAR_IDLE;
		}
		else if (score == 6 && chk_state == 3)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 48, 55, 8), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next3"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 3;
			ku_state = CHAR_IDLE;
		}
		else if (score == 6 && chk_state == 4)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 255, 255, 255), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next4"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 4;
			ku_state = CHAR_IDLE;
		}
		else if (score == 5 && chk_state == 5)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 255, 255, 255), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next5"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 5;
			ku_state = CHAR_IDLE;
		}
		else if (score == 6 && chk_state == 6)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 255, 255, 255), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next6"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 6;
			ku_state = CHAR_IDLE;
		}
		else if (score == 5 && chk_state == 7)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 255, 255, 255), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next7"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 7;
			ku_state = CHAR_IDLE;
		}
		else if (score == 6 && chk_state == 8)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 226, 52, 69), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-next8"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 8;
			ku_state = CHAR_IDLE;
		}

		else if (score == 6 && chk_state == 9)
		{
			//font->draw("YOU WIN!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 107, 65, 44), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textwin"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/sound-win.mp3");
			finish = 9;
			ku_state = CHAR_IDLE;
		}


		if (count == 5)
		{
			//font->draw("YOU LOSE!", irr::core::recti(0, 0, 960, 640), irr::video::SColor(255, 98, 51, 20), true, true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("textlose"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-menu"), true);
			g_psd->setPsdVisible(g_psd->getPsdIdByPrefix("btn-replay"), true);
			g_snd->Play("source/sound/paka.mp3",0,false);
			ku_state = CHAR_IDLE;
		}

		if (g_input->getTouchState(0) == irr::EKS_RELEASED)		//เมื่อกด tap และจะเช็คว่า tap ที่ตำแหน่งอะไร 			
		{
			int x = g_input->getTouchX(0);
			int y = g_input->getTouchY(0);

			if (g_psd->hit(x, y, irr::core::stringc("btn-next1 hud")))
				state = STATE_02;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next2 hud")))
				state = STATE_03;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next3 hud")))
				state = STATE_04;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next4 hud")))
				state = STATE_05;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next5 hud")))
				state = STATE_06;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next6 hud")))
				state = STATE_07;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next7 hud")))
				state = STATE_08;
			if (g_psd->hit(x, y, irr::core::stringc("btn-next8 hud")))
				state = STATE_09;

			if (g_psd->hit(x, y, irr::core::stringc("btn-replay1 hud")))
				state = STATE_01;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay2 hud")))
				state = STATE_02;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay3 hud")))
				state = STATE_03;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay4 hud")))
				state = STATE_04;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay5 hud")))
				state = STATE_05;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay6 hud")))
				state = STATE_06;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay7 hud")))
				state = STATE_07;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay8 hud")))
				state = STATE_08;
			if (g_psd->hit(x, y, irr::core::stringc("btn-replay9 hud")))
				state = STATE_09;

		}
		
	}
	

}

void deinit()
{
	font->drop();
}

int main( int argc, char* args[] )
{
	arsa_SetFuncInit(init);
	arsa_SetFuncUpdate(update);
	arsa_SetFuncHud(hud);
	arsa_SetFuncDeInit(deinit);
	arsa_EasyStart();

	return 0;
}
