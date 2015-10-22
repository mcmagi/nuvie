/*
 *  PartyView.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Fri Aug 22 2003.
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
#include <cmath>
#include "nuvieDefs.h"

#include "Actor.h"
#include "Party.h"
#include "Player.h"
#include "GameClock.h"
#include "PartyView.h"
#include "Font.h"
#include "Weather.h"
#include "Script.h"
#include "MsgScroll.h"
#include "Event.h"
#include "Configuration.h"
#include "CommandBar.h"
#include "UseCode.h"
#include "MapWindow.h"
#include "SunMoonStripWidget.h"

extern GUI_status inventoryViewButtonCallback(void *data);
extern GUI_status actorViewButtonCallback(void *data);

#define U6 Game::get_game()->get_game_type()==NUVIE_GAME_U6
#define SE Game::get_game()->get_game_type()==NUVIE_GAME_SE
#define MD Game::get_game()->get_game_type()==NUVIE_GAME_MD

static const uint8 ACTION_BUTTON = 3;

PartyView::PartyView(Configuration *cfg) : View(cfg)
{
 player = NULL; view_manager = NULL;
 party_view_targeting = false;
 row_offset = 0;
 sun_moon_widget = NULL;
}

PartyView::~PartyView()
{

}

bool PartyView::init(void *vm, uint16 x, uint16 y, Font *f, Party *p, Player *pl, TileManager *tm, ObjManager *om)
{
 View::init(x,y,f,p,tm,om);
 // PartyView is 8px wider than other Views, for the arrows
 // ...and 3px taller, for the sky (SB-X)
 if(U6)
   SetRect(area.x, area.y, area.w+8, area.h+3);
 else
   SetRect(area.x, area.y, area.w, area.h);

 view_manager = vm;
 player = pl;

 if(U6)
 {
   sun_moon_widget = new SunMoonStripWidget(player, tile_manager);
   sun_moon_widget->init(area.x,area.y);
   AddWidget(sun_moon_widget);
 }

 config->value("config/input/party_view_targeting", party_view_targeting, false);

 return true;
}

GUI_status PartyView::MouseUp(int x,int y,int button)
{
 x -= area.x;
 y -= area.y;

 if(y < 18 && U6) // clicked on skydisplay
   return GUI_PASS;
 if(y < 4 && MD)
   return GUI_PASS;

 int rowH = 16;
 if(MD)
     rowH = 24;

 uint8 party_size = party->get_party_size();
 if(SE)
 {
     if(party_size > 7) party_size = 7;
 }
 else if(party_size > 5) party_size = 5; // can only display/handle 5 at a time

 SDL_Rect arrow_rects_U6[2] = {{0,18,8,8},{0,90,8,8}};
 SDL_Rect arrow_rects[2] = {{0,6,7,8},{0,102,7,8}};
 SDL_Rect arrow_up_rect_MD[1] = {{0,15,7,8}};

//FIXME SDL2
// if(button == SDL_BUTTON_WHEELUP || HitRect(x,y,U6? arrow_rects_U6[0]: (MD ? arrow_up_rect_MD[0] : arrow_rects[0]))) //up arrow hit rect
//   {
//    if(up_arrow())
//      Redraw();
//    return GUI_YUM;
//   }
//  if(button == SDL_BUTTON_WHEELDOWN || HitRect(x,y,U6? arrow_rects_U6[1]: arrow_rects[1])) //down arrow hit rect
//   {
//    if(down_arrow())
//      Redraw();
//    return GUI_YUM;
//   }

 int x_offset = 7;
 int y_offset = 18;
 if(SE)
 {
     x_offset = 6;
     y_offset = 2;
 }
 else if(MD)
     y_offset = 4;
 if(y > party_size * rowH + y_offset-1) // clicked below actors
   return GUI_YUM;

 if(x >= x_offset)
  {
   Event *event = Game::get_game()->get_event();
   CommandBar *command_bar = Game::get_game()->get_command_bar();

   if(button == ACTION_BUTTON && event->get_mode() == MOVE_MODE
      && command_bar->get_selected_action() > 0) // Exclude attack mode too
   {
      if(command_bar->try_selected_action() == false) // start new action
         return GUI_PASS; // false if new event doesn't need target
   }
   if((party_view_targeting || (button == ACTION_BUTTON
      && command_bar->get_selected_action() > 0)) && event->can_target_icon())
   {
      x += area.x;
      y += area.y;
      Actor *actor = get_actor(x, y);
      if(actor)
      {
         event->select_actor(actor);
         return GUI_YUM;
      }
   }
   set_party_member(((y - y_offset) / rowH) + row_offset);
   if(x >= x_offset + 17) // clicked an actor name
     {
      actorViewButtonCallback(view_manager);
     }
   else // clicked an actor icon
     {
      inventoryViewButtonCallback(view_manager);
     }
  }
 return GUI_YUM;
}

Actor *PartyView::get_actor(int x, int y)
{
    x -= area.x;
    y -= area.y;

    uint8 party_size = party->get_party_size();
    int rowH = 16;
    int y_offset = 18;
    if(MD)
    {
        rowH = 24; y_offset = 0;
    }
    if(SE)
    {
        y_offset = 2;
        if(party_size > 7) party_size = 7;
    }
    else if(party_size > 5) party_size = 5; // can only display/handle 5 at a time

    if(y > party_size * rowH + y_offset) // clicked below actors
      return NULL;

    if(x >= 8)
     {
    	return party->get_actor(((y - y_offset) / rowH) + row_offset);
     }

    return NULL;
}

bool PartyView::drag_accept_drop(int x, int y, int message, void *data)
{
	GUI::get_gui()->force_full_redraw();
	DEBUG(0,LEVEL_DEBUGGING,"PartyView::drag_accept_drop()\n");
	if(message == GUI_DRAG_OBJ)
	{
		MsgScroll *scroll = Game::get_game()->get_scroll();

		Obj *obj = (Obj*)data;
		Actor *actor = get_actor(x, y);
		if(actor)
		{
			Event *event = Game::get_game()->get_event();
			event->display_move_text(actor, obj);
			if(!obj->is_in_inventory()
               && !Game::get_game()->get_map_window()->can_get_obj(actor, obj))
			{
				Game::get_game()->get_scroll()->message("\n\nblocked\n\n");
				return false;
			}
			if((!Game::get_game()->get_usecode()->has_getcode(obj)
			    || Game::get_game()->get_usecode()->get_obj(obj, actor))
			   && event->can_move_obj_between_actors(obj, player->get_actor(), actor))
			{
				if(actor == player->get_actor()) // get
					player->subtract_movement_points(3);
				else // get plus move
					player->subtract_movement_points(8);
				if(!obj->is_in_inventory() &&
				   obj_manager->obj_is_damaging(obj, Game::get_game()->get_player()->get_actor()))
					return false;
				DEBUG(0,LEVEL_DEBUGGING,"Drop Accepted\n");
				return true;
			}
		}
		scroll->display_string("\n\n");
	    scroll->display_prompt();
	}

	Redraw();

	DEBUG(0,LEVEL_DEBUGGING,"Drop Refused\n");
	return false;
}

void PartyView::drag_perform_drop(int x, int y, int message, void *data)
{
 DEBUG(0,LEVEL_DEBUGGING,"InventoryWidget::drag_perform_drop()\n");
 Obj *obj;

 if(message == GUI_DRAG_OBJ)
   {
    DEBUG(0,LEVEL_DEBUGGING,"Drop into inventory.\n");
    obj = (Obj *)data;

	Actor *actor = get_actor(x, y);
	if(actor)
	{
		obj_manager->moveto_inventory(obj, actor);
	}
    MsgScroll *scroll = Game::get_game()->get_scroll();
    scroll->display_string("\n\n");
    scroll->display_prompt();

    Redraw();
   }

 return;
}

void PartyView::Display(bool full_redraw)
{

 if(full_redraw || update_display || MD || Game::get_game()->is_original_plus_full_map())
  {
   uint8 i;
   uint8 hp_text_color;
   Actor *actor;
   Tile *actor_tile;
   char *actor_name;
   char hp_string[4];
   uint8 party_size = party->get_party_size();
   int rowH = 16;
   if(MD)
      rowH = 24;
   update_display = false;
   uint8 end_offset = row_offset + 5;
   if(MD)
      fill_md_background(area);
   else
      screen->fill(bg_color, area.x, area.y, area.w, area.h);
   //if(U6)
     // display_sun_moon_strip();

   display_arrows();

   if(SE)
	   end_offset = row_offset + 7;
   if(end_offset > party_size)
	   end_offset = party_size;

   for(i=row_offset;i<end_offset;i++)
     {
      actor = party->get_actor(i);
      actor_tile = tile_manager->get_tile(actor->get_downward_facing_tile_num());

      int x_offset = 8;
      int y_offset = 18;
      hp_text_color = 0; //standard text color

      if(U6)
        hp_text_color = 0x48; //standard text color 
      if (SE)
      {
        x_offset = 6; y_offset = 1;
      }
      if(MD)
      {
        x_offset = 8;
        y_offset = 6;
        GameClock *clock = Game::get_game()->get_clock();
        if(clock->get_purple_berry_counter(actor->get_actor_num()) > 0)
        {
          screen->blit(area.x+x_offset+16,area.y + y_offset + (i-row_offset)*rowH,tile_manager->get_tile(TILE_MD_PURPLE_BERRY_MARKER)->data,8,16,16,16,true);
        }
        if(clock->get_green_berry_counter(actor->get_actor_num()) > 0)
        {
          screen->blit(area.x+x_offset+32,area.y + y_offset + (i-row_offset)*rowH,tile_manager->get_tile(TILE_MD_GREEN_BERRY_MARKER)->data,8,16,16,16,true);
        }
        if(clock->get_brown_berry_counter(actor->get_actor_num()) > 0)
        {
          screen->blit(area.x+x_offset+32,area.y + y_offset + (i-row_offset)*rowH,tile_manager->get_tile(TILE_MD_BROWN_BERRY_MARKER)->data,8,16,16,16,true);
        }

      }

      screen->blit(area.x+x_offset,area.y + y_offset + (i-row_offset)*rowH,actor_tile->data,8,16,16,16,true);
      actor_name = party->get_actor_name(i);

      if(SE)
      {
        x_offset = 4; y_offset = 0;
      }
      if(MD)
        y_offset = -3;
      // FIXME: Martian Dreams text is somewhat center aligned
      font->drawString(screen, actor_name, area.x + x_offset + 24, area.y + y_offset + (i-row_offset) * rowH + 8);
      sprintf(hp_string,"%3d",actor->get_hp());
      hp_text_color = actor->get_hp_text_color();
      if(SE)
      {
        x_offset = -7; y_offset = 3;
      }
      if(MD)
      {
        x_offset = -16; y_offset = 14;
      }
      font->drawString(screen, hp_string, strlen(hp_string), area.x + x_offset + 112, area.y + y_offset + (i-row_offset) * rowH, hp_text_color, 0);
     }
   DisplayChildren(full_redraw);
   screen->update(area.x, area.y, area.w, area.h);
  }

 return;
}

bool PartyView::up_arrow()
{
    if(row_offset > 0)
    {
        row_offset--;
        return(true);
    }
    return(false);
}


bool PartyView::down_arrow()
{
    if((row_offset+ (SE? 7:5)) < party->get_party_size())
    {
        row_offset++;
        return(true);
    }
    return(false);
}


void PartyView::display_arrows()
{
    int x_offset = 0; int y_offset = 0;
    if(SE || MD)
    {
        x_offset = 2;
        y_offset = 12;
    }
    uint8 max_party_size = 5;
    uint8 party_size = party->get_party_size();
    if(SE)
        max_party_size = 7;
    if(party_size <= max_party_size) // reset
        row_offset = 0;

    if((party_size - row_offset) > max_party_size) // display bottom arrow
        font->drawChar(screen, 25, area.x - x_offset, area.y + 90 + y_offset);
    if(MD)
        y_offset = 3;
    if(row_offset > 0) // display top arrow
        font->drawChar(screen, 24, area.x - x_offset, area.y + 18 - y_offset);
}
// </SB-X> 
