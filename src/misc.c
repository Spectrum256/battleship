/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * misc.c
 * Copyright (C) Bruno Ramalhete 2015-2016 <bram.512@gmail.com>
 * 
 * misc.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * misc.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define N_(String) (String)
#endif


#define CONFIG_FILE "/.battleship.cfg"


extern void end_dialog ();

extern gint NumberOfRows, NumberOfColumns, NumberOfShips, DifficultyLimit;
extern gint my_matrix[15][15], his_matrix[15][15], found_matrix[15][15], found_row[15], found_column[15];
extern gint help;
gint search_matrix[15][15], search_row[15], search_column[15], tip_row[15], tip_column[15];
extern GtkButton *my_position[15][15], *his_position[15][15];
GtkWidget *my_image[15][15], *his_image[15][15];
extern GtkFrame *my_frame, *his_frame;
extern GtkLabel *my_label, *his_label;
extern GdkPixbuf *pixbuf_water, *pixbuf_my_ship, *pixbuf_his_ship, *pixbuf_hit_ship, *pixbuf_unknown;
gboolean my_blocked = FALSE, his_blocked = FALSE;
gint next_row = -1, next_column = -1, next_direction;
gint my_found_ships, his_found_ships;
gint my_time = 0, my_time_reference = 0;
gint his_time = 0, his_time_reference = 0;
gint timeout_id;


gint milliseconds () {
	struct timeval tv;
	
    gettimeofday (&tv, NULL);
    return (gint)(tv.tv_sec * 1000 + (tv.tv_usec / 1000));
}


gboolean update_time () {
	extern GtkLabel *his_time_label;
	char *string = malloc (6);
	gint current_time, minutes, seconds;
	
	current_time = my_time + milliseconds () - my_time_reference;
	minutes = current_time / 1000 / 60;
	seconds = current_time / 1000 - minutes * 60;
	
	if (seconds < 10)
		sprintf (string, "%2d:0%1d", minutes, seconds);
	else
		sprintf (string, "%2d:%2d", minutes, seconds);
	gtk_label_set_text (his_time_label, string);
	free (string);
	
	return TRUE;
}


void block_my () {
	extern gulong my_handler_id[15][15];
	gint row, column;

	if (my_blocked == FALSE) {
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++)
				g_signal_handler_block (G_OBJECT (my_position[row][column]), my_handler_id[row][column]);
		my_blocked = TRUE;
	}
}


void unblock_my () {
	extern gulong my_handler_id[15][15];
	gint row, column;

	if (my_blocked == TRUE) {
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++)
				g_signal_handler_unblock (G_OBJECT (my_position[row][column]), my_handler_id[row][column]);
		my_blocked = FALSE;
	}	
}


void block_his () {
	extern gulong his_handler_id[15][15];
	gint row, column;

	if (his_blocked == FALSE) {
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++)
				g_signal_handler_block (G_OBJECT (his_position[row][column]), his_handler_id[row][column]);
		his_blocked = TRUE;
	}
}


void unblock_his () {
	extern gulong his_handler_id[15][15];
	gint row, column;

	if (his_blocked == TRUE) {
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++)
				g_signal_handler_unblock (G_OBJECT (his_position[row][column]), his_handler_id[row][column]);
		his_blocked = FALSE;
	}
}


gint count_ships_row (gint matrix[15][15], gint row, gint value) {
	gint ships = 0, j;
	
	for (j=0;j<NumberOfColumns;j++)
		if (matrix[row][j] == value)
			ships++;
	
	return ships;
}


gint count_ships_column (gint matrix[15][15], gint column, gint value) {
	gint ships = 0, j;
	
	for (j=0;j<NumberOfRows;j++) {
		if (matrix[j][column] == value)
			ships++;
	}

	return ships;
}


//~ void print_matrix (gint matrix[15][15], gint vector1[15], gint vector2[15]) {
	//~ gint row, column;

	//~ g_print ("       ");
	//~ for (column=0;column<NumberOfColumns;column++)
		//~ g_print ("%4d ", vector2[column]);
	//~ g_print ("\n");
	//~ for (row=0;row<NumberOfRows;row++) {
		//~ g_print ("%4d   ", vector1[row]);
		//~ for (column=0;column<NumberOfColumns;column++)
			//~ g_print ("%4d ", matrix[row][column]);
		//~ g_print ("\n");
	//~ }
//~ }


void initialize() {
	extern GtkLabel *my_time_label, *his_time_label;
	gint row, column;
	char *string;

	for (row=0;row<NumberOfRows;row++)
		for (column=0;column<NumberOfColumns;column++) {
			my_matrix[row][column] = 0;
			his_matrix[row][column] = 0;
			found_matrix[row][column] = -10;
			search_matrix[row][column] = -10;
			
			gtk_button_set_label (my_position[row][column], "");
			gtk_button_set_label (his_position[row][column], "");
			my_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
			gtk_button_set_image (my_position[row][column], my_image[row][column]);
			his_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
			gtk_button_set_image (his_position[row][column], his_image[row][column]);
		}
	
	string = malloc (strlen(_("The Enemy's Ships")) + 6);
	sprintf (string, _("The Enemy's Ships %d/%d"), 0, NumberOfShips);
	gtk_frame_set_label (his_frame, string);
	free (string);

	string = malloc (strlen(_("My Ships")) + 6);
	sprintf (string, _("My Ships %d/%d"), 0, NumberOfShips);
	gtk_frame_set_label (my_frame, string);
	free (string);

	gtk_label_set_text (my_time_label, "0:00,000");
	gtk_label_set_text (his_time_label, "0:00");
	
	gtk_label_set_text (my_label, _("Place ships."));
	gtk_label_set_text (his_label, "");
	
	for (row=0;row<NumberOfRows;row++) {
		found_row[row] = -1;
		search_row[row] = -1;
		tip_row[row] = -1;
	}
	for (column=0;column<NumberOfColumns;column++) {
		found_column[column] = -1;
		search_column[column] = -1;
		tip_column[column] = -1;
	}
	
	next_row = -1;
	next_column = -1;
	my_found_ships = 0;
	his_found_ships = 0;
	my_time = 0;
	his_time = 0;

	block_his ();
	unblock_my ();
}


void fill_his_matrix() {
	gint i, j, row, column;
	gint ships, offset;
	
	srand((unsigned) time(NULL)); 
	
	for (i=1;i<=NumberOfShips;i++) {
		offset = (rand() % NumberOfShips == 0 ? 1 : 0);
		offset += 2 * (ceil ((float)i / (float)(NumberOfRows < NumberOfColumns ? NumberOfRows : NumberOfColumns)) - 1);
		do {
			do {
				row = rand() % NumberOfRows;
				column = rand() % NumberOfColumns;
			} while (his_matrix[row][column] == 1);
				
			ships = 0;
			for (j=0;j<NumberOfRows;j++)
				ships += his_matrix[j][column];
			for (j=0;j<NumberOfColumns;j++)
				ships += his_matrix[row][j];
		} while (ships >= DifficultyLimit + offset);
		
		his_matrix[row][column] = 1;
	}
}


void his_turn () {
	extern GtkLabel *my_time_label;
	gint j, row, column, start_row, start_column;
	gint ships, ships_row, ships_column;
	char *string;
	gboolean hit = FALSE, flag, ready;
	gint minutes; 
	float seconds;

	his_time_reference = milliseconds ();

	if (next_row == -1 && next_column == -1) {
		do {
			row = rand() % NumberOfRows;
			column = rand() % NumberOfColumns;
		} while (search_matrix[row][column] != -10);
		if (rand() % 2 == 0) {
			next_row = row;
			next_direction = 1;
		}
		else {
			next_column = column;
			next_direction = 0;
		}
	}
	else if (next_row != -1 && next_column != -1) {
		row = next_row;
		column = next_column;
	}
	
	if (my_matrix[row][column] != 1) {
		my_image[row][column] = NULL;
		gtk_button_set_image (my_position[row][column], my_image[row][column]);
		search_matrix[row][column] = count_ships_row (my_matrix, row, 1) + count_ships_column (my_matrix, column, 1);
		tip_row[row] = 1;
		tip_column[column] = 1;
		string = malloc (3);
		sprintf (string, "%d", search_matrix[row][column]);
		gtk_button_set_label (my_position[row][column], string);
		free (string);
		
		string = malloc (strlen (_("Last turn: (+99, +99).")));
		sprintf (string, _("Last turn: (%d, %d)."), row+1, column+1);
		gtk_label_set_text (my_label, string);
		free (string);
	}
	else {
		gtk_button_set_label (my_position[row][column], "");
		my_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_hit_ship);
		gtk_button_set_image (my_position[row][column], my_image[row][column]);
		search_matrix[row][column] = -1;
		my_found_ships++;
		string = malloc (strlen (_("My Ships +99/+99")));
		sprintf (string, _("My Ships %d/%d"), my_found_ships, NumberOfShips);
		gtk_frame_set_label (my_frame, string);
		free (string);
		
		string = malloc (strlen (_("Ship hit: (+99, +99).")));
		sprintf (string, _("Ship hit: (%d, %d)."), row+1, column+1);
		gtk_label_set_text (my_label, string);
		free (string);
		
		hit = TRUE;
		if (my_found_ships == NumberOfShips) {
			my_found_ships = 0;
			end_dialog (_("\nYou lost.\n"));
			return;
		}
	}
	
	for (row=0;row<NumberOfRows;row++)
		for (column=0;column<NumberOfColumns;column++)
			if (search_matrix[row][column] >= 0) {
				ships_row = count_ships_row (search_matrix, row, -1);
				ships_column = count_ships_column (search_matrix, column, -1);
				if (ships_row + ships_column == search_matrix[row][column]) {
					for (j=0;j<NumberOfRows;j++)
						if (search_matrix[j][column] == -10) {  //uninitialized
							search_matrix[j][column] = -2;  //no ship on this position
							if (my_matrix[j][column] != 1) {
								my_image[j][column] = gtk_image_new_from_pixbuf (pixbuf_water);
								gtk_button_set_label (my_position[j][column], "");
								gtk_button_set_image (my_position[j][column], my_image[j][column]);
							}
						}
					for (j=0;j<NumberOfColumns;j++)
						if (search_matrix[row][j] == -10) {  //uninitialized
							search_matrix[row][j] = -2;  //no ship on this position
							if (my_matrix[row][j] != 1) {
								my_image[row][j] = gtk_image_new_from_pixbuf (pixbuf_water);
								gtk_button_set_label (my_position[row][j], "");
								gtk_button_set_image (my_position[row][j], my_image[row][j]);
							}
						}
					search_row[row] = ships_row;
					search_column[column] = ships_column;
				}
			}
	
	do {
		ready = TRUE;
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++)
				if (search_matrix[row][column] >= 0) {
					if (search_row[row] == -1 && search_column[column] >= 0) {
						search_row[row] = search_matrix[row][column] - search_column[column];
						if (search_row[row] == count_ships_row (search_matrix, row, -1))
							for (j=0;j<NumberOfColumns;j++)
								if (search_matrix[row][j] == -10) {
									search_matrix[row][j] = -2;
									if (my_matrix[row][j] != 1) {
										my_image[row][j] = gtk_image_new_from_pixbuf (pixbuf_water);
										gtk_button_set_label (my_position[row][j], "");
										gtk_button_set_image (my_position[row][j], my_image[row][j]);
									}
										gtk_button_set_label (my_position[row][j], "");
								}
						ready = FALSE;
					}
					else if (search_row[row] >= 0 && search_column[column] == -1) {
						search_column[column] = search_matrix[row][column] - search_row[row];
						if (search_column[column] == count_ships_column (search_matrix, column, -1))
							for (j=0;j<NumberOfRows;j++)
								if (search_matrix[j][column] == -10) {
									search_matrix[j][column] = -2;
									if (my_matrix[j][column] != 1) {
										my_image[j][column] = gtk_image_new_from_pixbuf (pixbuf_water);
										gtk_button_set_label (my_position[j][column], "");
										gtk_button_set_image (my_position[j][column], my_image[j][column]);
							}
								}
						ready = FALSE;
					}					
				}
	} while (ready ==FALSE);
	
	for (row=0;row<NumberOfRows;row++)
		if (search_row[row] >= 0) {
			if (search_row[row] == count_ships_row (search_matrix, row, -1))
				for (j=0;j<NumberOfColumns;j++)
					if (search_matrix[row][j] == -10) {
						search_matrix[row][j] = -2;
						if (my_matrix[row][j] != 1){
							my_image[row][j] = gtk_image_new_from_pixbuf (pixbuf_water);
							gtk_button_set_label (my_position[row][j], "");
							gtk_button_set_image (my_position[row][j], my_image[row][j]);
						}
					}
		}
	
	for (column=0;column<NumberOfColumns;column++)
		if (search_column[column] >= 0) {
			if (search_column[column] == count_ships_column (search_matrix, column, -1))
				for (j=0;j<NumberOfRows;j++)
					if (search_matrix[j][column] == -10) {
						search_matrix[j][column] = -2;
						if (my_matrix[j][column] != 1) {
							my_image[j][column] = gtk_image_new_from_pixbuf (pixbuf_water);
							gtk_button_set_label (my_position[j][column], "");
							gtk_button_set_image (my_position[j][column], my_image[j][column]);
						}
					}
		}
		
	ready = FALSE;
	for (row=0;row<NumberOfRows;row++) {
		ships = count_ships_row (search_matrix, row, -1);
		if (search_row[row] > 0)
			if (count_ships_row (search_matrix, row, -10) == search_row[row] - ships && search_row[row] - ships > 0) {
				next_row = row;
				for (column=0;column<NumberOfColumns;column++)
					if (search_matrix[row][column] == -10)
						next_column = column;
				ready = TRUE;
			}
	}
	for (column=0;column<NumberOfColumns;column++) {
		ships = count_ships_column (search_matrix, column, -1);
		if (search_column[column] > 0)
			if (count_ships_column (search_matrix, column, -10) == search_column[column] - ships && search_column[column] - ships > 0) {
				next_column = column;
				for (row=0;row<NumberOfRows;row++)
					if (search_matrix[row][column] == -10)
						next_row = row;
				ready = TRUE;
			}
	}

	if (ready == FALSE) {
	
		if (next_direction == 1) {  //stay in the same row
			if (count_ships_row (search_matrix, next_row, -10) == 0) {
				flag = FALSE;
				for (j=0;j<NumberOfRows;j++)
					if (tip_row[j] == -1)
						flag = TRUE;
			
				if (flag == TRUE)
					do {
						next_row = rand() % NumberOfRows;
					} while (tip_row[next_row] != -1);
				else
					next_direction = 2;
			}
			
			flag = FALSE;
			for (j=0;j<NumberOfColumns;j++)
				if (tip_column[j] == -1 && search_matrix[next_row][j] == -10)
					flag = TRUE;
			
			if (flag == TRUE) {
				do {
					next_column = rand() % NumberOfColumns;
				} while (tip_column[next_column] != -1 || search_matrix[next_row][next_column] != -10);
				next_direction = 0;
			}
			else {
				flag = FALSE;
				for (j=0;j<NumberOfRows;j++)
					if (tip_row[j] == -1)
						flag = TRUE;
			
				if (flag == TRUE) {
					do {
						next_row = rand() % NumberOfRows;
					} while (tip_row[next_row] != -1);
					do {
						next_column = rand() % NumberOfColumns;
					} while (search_matrix[next_row][next_column] != -10);
					next_direction = 1;
				}
				else
					next_direction = 2;
			}
		}
		else if (next_direction == 0) {  //stay in the same column
			if (count_ships_column (search_matrix, next_column, -10) == 0) {
				flag = FALSE;
				for (j=0;j<NumberOfColumns;j++)
					if (tip_column[j] == -1)
						flag = TRUE;
			
				if (flag == TRUE) {
					do {
						next_column = rand() % NumberOfColumns;
					} while (tip_column[next_column] != -1);
					next_direction = 0;
				}
				else
					next_direction = 2;
			}
			
			flag = FALSE;
			for (j=0;j<NumberOfRows;j++)
				if (tip_row[j] == -1 && search_matrix[j][next_column] == -10)
					flag = TRUE;
			
			if (flag == TRUE) {
				do {
					next_row = rand() % NumberOfRows;
				} while (tip_row[next_row] != -1 || search_matrix[next_row][next_column] != -10);
				next_direction = 1;
			}
			else {
				flag = FALSE;
				for (j=0;j<NumberOfColumns;j++)
					if (tip_column[j] == -1)
						flag = TRUE;
			
				if (flag == TRUE) {
					do {
						next_column = rand() % NumberOfColumns;
					} while (tip_column[next_column] != -1);
					do {
						next_row = rand() % NumberOfRows;
					} while (search_matrix[next_row][next_column] != -10);
					next_direction = 0;
				}
				else
					next_direction = 2;			
			}
		}
		
		if (next_direction == 2) {  //calculate probability
			ships_row = NumberOfRows;
			start_row = rand() % NumberOfRows;
			for (j=start_row;j<start_row+NumberOfRows;j++) {
				row = j % NumberOfRows;
				ships = count_ships_row (search_matrix, row, -10);
				if ( ships > 0 && ships < ships_row) {
					ships_row = ships;
					next_row = row;
				}
			}

			ships_column = NumberOfColumns;
			start_column = rand() % NumberOfColumns;
			for (j=start_column;j<start_column+NumberOfColumns;j++) {
				column = j % NumberOfColumns;
				ships = count_ships_column (search_matrix, column, -10);
				if ( ships >0 && ships < ships_column) {
					ships_column = ships;
					next_column = column;
				}
			}
			
			if (ships_row < ships_column) {
				do {
					next_column = rand() % NumberOfColumns;
				} while (search_matrix[next_row][next_column] != -10);
			}
			else {
				do {
					next_row = rand() % NumberOfRows;
				} while (search_matrix[next_row][next_column] != -10);
			}
		}
		
	}
	
	his_time += milliseconds () - his_time_reference;
	minutes = his_time / 1000 / 60;
	seconds = (float)his_time / 1000 - (float)minutes * 60;
	
	string = malloc (10);
	if (seconds < 10)
		sprintf (string, "%2d:0%1.3f", minutes, seconds);
	else
		sprintf (string, "%2d:%2.3f", minutes, seconds);
	gtk_label_set_text (my_time_label, string);
	free (string);

	if (hit == TRUE)
		his_turn ();
}


void my_turn (gint row, gint column) {
	gint j, ships_row, ships_column;
	char *string;
	gboolean hit = FALSE, ready;
	
	my_time += milliseconds () - my_time_reference;
	
	block_his ();
	
	if (found_matrix[row][column] >= -1) {
		string = malloc (strlen (_("Invalid: (+99, +99).")));
		sprintf (string, _("Invalid: (%d, %d)."), row, column);
		gtk_label_set_text (his_label, string);
		free (string);
		unblock_his ();
		return;
	}
	else if (his_matrix[row][column] != 1) {
		his_image[row][column] = NULL;
		gtk_button_set_image (his_position[row][column], his_image[row][column]);
		found_matrix[row][column] = count_ships_row (his_matrix, row, 1) + count_ships_column (his_matrix, column, 1);
		string = malloc (3);
		sprintf (string, "%d", found_matrix[row][column]);
		gtk_button_set_label (his_position[row][column], string);
		free (string);
		
		string = malloc (strlen (_("Last turn: (+99, +99).")));
		sprintf (string, _("Last turn: (%d, %d)."), row+1, column+1);
		gtk_label_set_text (his_label, string);
		free (string);
	}
	else {
		gtk_button_set_label (his_position[row][column], "");
		his_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_hit_ship);
		gtk_button_set_image (his_position[row][column], his_image[row][column]);
		found_matrix[row][column] = -1;
		his_found_ships++;
		string = malloc (strlen (_("The Enemy's Ships +99/+99")));
		sprintf (string, _("The Enemy's Ships %d/%d"), his_found_ships, NumberOfShips);
		gtk_frame_set_label (his_frame, string);
		free (string);
		
		string = malloc (strlen (_("Ship hit: (+99, +99).")));
		sprintf (string, _("Ship hit: (%d, %d)."), row+1, column+1);
		gtk_label_set_text (his_label, string);
		free (string);
	
		hit = TRUE;
	}
	
	if (help >= 1) {
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++)
				if (found_matrix[row][column] >= 0) {
					ships_row = count_ships_row (found_matrix, row, -1);
					ships_column = count_ships_column (found_matrix, column, -1);
					if (ships_row + ships_column == found_matrix[row][column]) {
						for (j=0;j<NumberOfRows;j++)
							if (found_matrix[j][column] == -10) {
								gtk_button_set_label (his_position[j][column], "");
								his_image[j][column] = gtk_image_new_from_pixbuf (pixbuf_water);
								gtk_button_set_image (his_position[j][column], his_image[j][column]);
								found_matrix[j][column] = -2;
							}
						for (j=0;j<NumberOfColumns;j++)
							if (found_matrix[row][j] == -10) {
								gtk_button_set_label (his_position[row][j], "");
								his_image[row][j] = gtk_image_new_from_pixbuf (pixbuf_water);
								gtk_button_set_image (his_position[row][j], his_image[row][j]);
								found_matrix[row][j] = -2;
							}
						found_row[row] = ships_row;
						found_column[column] = ships_column;
					}
				}
	}
		
	if (help >= 2) {
		do {
			ready = TRUE;
			for (row=0;row<NumberOfRows;row++)
				for (column=0;column<NumberOfColumns;column++)
					if (found_matrix[row][column] >= 0) {
						if (found_row[row] == -1 && found_column[column] >= 0) {
							found_row[row] = found_matrix[row][column] - found_column[column];
							if (found_row[row] == count_ships_row (found_matrix, row, -1))
								for (j=0;j<NumberOfColumns;j++)
									if (found_matrix[row][j] == -10) {
										gtk_button_set_label (his_position[row][j], "");
										his_image[row][j] = gtk_image_new_from_pixbuf (pixbuf_water);
										gtk_button_set_image (his_position[row][j], his_image[row][j]);
										found_matrix[row][j] = -2;
									}
							ready = FALSE;
						}
						if (found_row[row] >= 0 && found_column[column] == -1) {
							found_column[column] = found_matrix[row][column] - found_row[row];
							if (found_column[column] == count_ships_column (found_matrix, column, -1))
								for (j=0;j<NumberOfRows;j++)
									if (found_matrix[j][column] == -10) {
										gtk_button_set_label (his_position[j][column], "");
										his_image[j][column] = gtk_image_new_from_pixbuf (pixbuf_water);
										gtk_button_set_image (his_position[j][column], his_image[j][column]);
										found_matrix[j][column] = -2;
									}
							ready = FALSE;
						}					
					}
		} while (ready ==FALSE);
	}
	
	if (help >= 3) {
		for (row=0;row<NumberOfRows;row++)
			if (found_row[row] >= 0) {
				if (found_row[row] == count_ships_row (found_matrix, row, -1))
					for (j=0;j<NumberOfColumns;j++)
						if (found_matrix[row][j] == -10) {
						gtk_button_set_label (his_position[row][j], "");
						his_image[row][j] = gtk_image_new_from_pixbuf (pixbuf_water);
						gtk_button_set_image (his_position[row][j], his_image[row][j]);
						found_matrix[row][j] = -2;
					}
			}
		
		for (column=0;column<NumberOfColumns;column++)
			if (found_column[column] >= 0) {
				if (found_column[column] == count_ships_column (found_matrix, column, -1))
					for (j=0;j<NumberOfRows;j++)
						if (found_matrix[j][column] == -10) {
							gtk_button_set_label (his_position[j][column], "");
							his_image[j][column] = gtk_image_new_from_pixbuf (pixbuf_water);
							gtk_button_set_image (his_position[j][column], his_image[j][column]);
							found_matrix[j][column] = -2;
						}
			}
	}
	
	
	if (his_found_ships < NumberOfShips) {
		if (hit == FALSE)
			his_turn();
		unblock_his ();
		my_time_reference = milliseconds ();
	}
	else {
		his_found_ships = 0;
		end_dialog (_("\nYou won.\n"));
		unblock_his ();
	}
}


void my_mark (gint row, gint column) {
	
	if (gtk_image_get_pixbuf (GTK_IMAGE (gtk_button_get_image (his_position[row][column]))) == pixbuf_unknown && found_matrix[row][column] < 0) {
		his_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_water);
		gtk_button_set_label (his_position[row][column], "");
		gtk_button_set_image (his_position[row][column], his_image[row][column]);
	}
	else if (gtk_image_get_pixbuf (GTK_IMAGE (gtk_button_get_image (his_position[row][column]))) == pixbuf_water) {
		gtk_button_set_label (his_position[row][column], "");
		his_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
		gtk_button_set_image (his_position[row][column], his_image[row][column]);
	}
}

void set_ship (gint row, gint column) {
	static gint set_ships = 0;
	
	if (set_ships < NumberOfShips) {
		if (my_matrix[row][column] != 1) {	
			gtk_button_set_label (my_position[row][column], "");
			my_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_my_ship);
			gtk_button_set_image (my_position[row][column], my_image[row][column]);
			set_ships++;
			my_matrix[row][column] = 1;
		}
		else {
			gtk_button_set_label (my_position[row][column], "");
			my_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
			gtk_button_set_image (my_position[row][column], my_image[row][column]);
			set_ships--;
			my_matrix[row][column] = 0;
		}
	}
		
	if (set_ships == NumberOfShips) {
		set_ships = 0;
		block_my ();
		fill_his_matrix();
		gtk_label_set_text (my_label, "");
	
		if (rand() % 2 == 0)
			his_turn();
		
		unblock_his ();
		
		my_time = 0;
		my_time_reference = milliseconds ();
		timeout_id = g_timeout_add (1000, update_time, NULL); 
	}
}


void save () {
	FILE *file;
	const gchar *home = getenv ("HOME");
	gchar *filename = malloc (strlen (home) + strlen (CONFIG_FILE) + 1);
	
	strcpy (filename, home);
	strcat (filename, CONFIG_FILE);
	
	file = fopen (filename, "w");

	if (file != NULL) {
		fprintf (file, "<NumberOfRows>\n%d\n</NumberOfRows>\n", NumberOfRows);
		fprintf (file, "<NumberOfColumns>\n%d\n</NumberOfColumns>\n", NumberOfColumns);
		fprintf (file, "<NumberOfShips>\n%d\n</NumberOfShips>\n", NumberOfShips);
		fprintf (file, "<DifficultyLimit>\n%d\n</DifficultyLimit>\n", DifficultyLimit);
		fprintf (file, "<Help>\n%d\n</Help>\n", help);
			
		fclose (file);
	}
	
	free (filename);
}


gint load (const gchar *keyword) {
	FILE *file;
	char *position, *tmp = malloc (256);
	char *line = malloc (256);
	char *start = malloc (16 + 3);
	char *stop = malloc (17 + 4);
	const char *home = getenv ("HOME");
	char *filename = malloc (strlen (home) + strlen (CONFIG_FILE) + 1);
	gint result = 0;
	
	strcpy (filename, home);
	strcat (filename, CONFIG_FILE);

	strcpy (start, "<");
	strcat (start, keyword);
	strcat (start, ">");
	strcpy (stop, "</");
	strcat (stop, keyword);
	strcat (stop, ">");

	file = fopen (filename, "r");
	
	if (file != NULL) {
		while ((fgets (line, 255, file)) != NULL) {
			if (strncmp (line, start, strlen (start)) == 0) break;
		}
		
		while ((fgets (line, 255, file)) != NULL) {
			if (strncmp (line, stop, strlen (stop)) != 0) {
				if ((position = strstr (line, "\n")) != NULL) {
					strncpy (tmp, line, position-line);
					strcpy (tmp + (position-line), ""); //strncpy does not append the trailing "\0"
				}
				result = atoi (tmp);
			}
			else break;
		}
		
		fclose (file);
	}
	
	free (start);
	free (stop);
	free (line);
	free (tmp);
	
	return result;
}
