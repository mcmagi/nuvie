#ifndef __SunMoonStripWidget_h__
#define __SunMoonStripWidget_h__

/*
 *  SunMoonStripWidget.h
 *  Nuvie
 *
 *  Created by Eric Fry on Fri Nov 29 2013.
 *  Copyright (c) 2013. All rights reserved.
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

#include "GUI_widget.h"
#include "TileManager.h"

class Player;

class SunMoonStripWidget : public GUI_Widget {

protected:
 TileManager *tile_manager;
 Player *player;

 public:
 SunMoonStripWidget(Player *p, TileManager *tm);
 ~SunMoonStripWidget();

 void init(sint16 x, sint16 y);
 void Display(bool full_redraw);

protected:
 virtual void display_sun_moon(Tile *tile, uint8 pos);
 void display_sun(uint8 hour, uint8 minute, bool eclipse);
 void display_moons(uint8 day, uint8 hour, uint8 minute=0);

 private:
 void display_surface_strip();
 void display_dungeon_strip();
};

#endif /* __SunMoonStripWidget_h__ */

