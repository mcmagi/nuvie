/*
 *  Background.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Aug 24 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <ctype.h>
#include <string>

#include "nuvieDefs.h"
#include "Configuration.h"
#include "U6misc.h"
#include "U6Lib_n.h"
#include "U6Bmp.h"
#include "Dither.h"
#include "Background.h"

Background::Background(Configuration *cfg) : GUI_Widget(NULL)
{
 config = cfg;
 config->value("config/GameType",game_type);

 bg_w = 0; bg_h = 0;
 background = NULL;
 x_off = Game::get_game()->get_game_x_offset();
 y_off = Game::get_game()->get_game_y_offset();


 Init(NULL, 0,0,Game::get_game()->get_screen()->get_width(),Game::get_game()->get_screen()->get_height());
}

Background::~Background()
{
 if(background)
  delete background;
}

bool Background::init()
{
 std::string filename;

 if(Game::get_game()->is_orig_style())
 {
	 switch(game_type)
	   {
		case NUVIE_GAME_U6 : config_get_path(config,"paper.bmp",filename);
							 background = (U6Shape *) new U6Bmp();
							 if(background->load(filename) == false)
							   return false;
							 break;

		case NUVIE_GAME_MD :
							 background = new U6Shape();
							 background->load_WoU_background(config, game_type);
							 break;

		case NUVIE_GAME_SE :
							 background = new U6Shape();
							 background->load_WoU_background(config, game_type);
							 break;
	   }

	 background->get_size(&bg_w,&bg_h);

	 Game::get_game()->get_dither()->dither_bitmap(background->get_data(),area.w, area.h, DITHER_NO_TRANSPARENCY);
 }
 return true;
}

void Background::Display(bool full_redraw)
{
 if(full_redraw || update_display)
   {
    update_display = false;

    screen->clear(area.x,area.y,area.w,area.h,NULL);

    if(Game::get_game()->is_orig_style())
    	screen->blit(x_off, y_off, background->get_data(), 8,  bg_w, bg_h, bg_w, true);

    screen->update(0,0,area.w,area.h);
   }

 return;
}

