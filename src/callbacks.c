/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * callbacks.c
 * Copyright (C) Bruno Ramalhete 2015-2016 <bram.512@gmail.com>
 * 
 * callbacks.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * callbacks.c is distributed in the hope that it will be useful, but
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

extern void initialize ();
extern void my_turn (gint row, gint column);
extern void my_mark (gint row, gint column);
extern void set_ship (gint row, gint column);
extern gint help, DifficultyLimit, NumberOfRows, NumberOfColumns, NumberOfShips;
GtkSpinButton *help_button, *difficulty_button, *rows_button, *columns_button, *ships_button;
extern GtkWidget *window;
GtkAdjustment *NumberOfShipsAdjustment, *DifficultyAdjustment;


gboolean on_his_position_clicked (GtkButton *button, GdkEventButton *event, gpointer data) {
	gint row, column;
	
	row = GPOINTER_TO_INT (data)/100;	
	column = GPOINTER_TO_INT (data)-row*100;

	switch (event->button) {
		case 1:
			my_turn (row, column);
			break;
		case 3:
			my_mark (row, column);
			break;
	}
	
	return FALSE;
}


gboolean on_my_position_clicked (GtkButton *position, gpointer data) {
	gint row, column;
	
	row = GPOINTER_TO_INT (data)/100;	
	column = GPOINTER_TO_INT (data)-row*100;

	set_ship (row, column);
	
	return FALSE;
}


void menu_remove_marks () {
	extern GdkPixbuf *pixbuf_water, *pixbuf_unknown;
	extern GtkButton *his_position[15][15];
	extern GtkWidget *his_image[15][15];
	gint row, column;
	
	for (row=0;row<NumberOfRows;row++)
		for (column=0;column<NumberOfColumns;column++)
			if (gtk_image_get_pixbuf (GTK_IMAGE (gtk_button_get_image (his_position[row][column]))) == pixbuf_water) {
				gtk_button_set_label (his_position[row][column], "");
				his_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
				gtk_button_set_image (his_position[row][column], his_image[row][column]);
			}
}


void menu_start () {
	initialize ();
}


void preferences_closed (GtkWidget *preferences_window) {
	extern void save ();
	extern GtkWidget *my_vbox, *his_vbox;
	extern GtkButton *my_position[15][15], *his_position[15][15];
	extern gulong my_handler_id[15][15], his_handler_id[15][15];
	extern GdkPixbuf *pixbuf_unknown;
	extern GtkWidget *my_image[15][15], *his_image[15][15];
	extern GtkWidget *my_table, *his_table;
	gboolean changed = FALSE;
	gint row, column;
	gint MaximumNumberOfShips;

	if (gtk_spin_button_get_value (rows_button) != NumberOfRows) {
		NumberOfRows = gtk_spin_button_get_value (rows_button);
		changed = TRUE;
	}
	
	if (gtk_spin_button_get_value (columns_button) != NumberOfColumns) {
		NumberOfColumns = gtk_spin_button_get_value (columns_button);
		changed = TRUE;
	}
	
	MaximumNumberOfShips = NumberOfRows + NumberOfColumns;
	if (gtk_spin_button_get_value (ships_button) != NumberOfShips) {
		NumberOfShips = gtk_spin_button_get_value (ships_button);
		changed = TRUE;
	}
	NumberOfShips = (NumberOfShips<MaximumNumberOfShips ? NumberOfShips : MaximumNumberOfShips);
	
	if (gtk_spin_button_get_value (difficulty_button) != DifficultyLimit) {
		DifficultyLimit = gtk_spin_button_get_value (difficulty_button);
		changed = TRUE;
	}
	
	if (gtk_spin_button_get_value (help_button) != help) {
		help = gtk_spin_button_get_value (help_button);
		changed = TRUE;
	}
	
	if (changed == TRUE) {
		save ();
	    gtk_window_resize (GTK_WINDOW (window), 1, 1);

		gtk_widget_destroy (my_table);
		my_table = gtk_table_new (NumberOfRows, NumberOfColumns, TRUE);
		gtk_box_pack_start (GTK_BOX (my_vbox), my_table, FALSE, FALSE, 0);
		gtk_widget_show (my_table);
		
		for (row=0;row<NumberOfRows;row++) {
			for (column=0;column<NumberOfColumns;column++) {
				my_position[row][column] = GTK_BUTTON (gtk_button_new_with_label (""));
				my_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
				gtk_button_set_image (my_position[row][column], my_image[row][column]);
				my_handler_id[row][column] = g_signal_connect (G_OBJECT (my_position[row][column]), "clicked", G_CALLBACK (on_my_position_clicked), GINT_TO_POINTER (row*100+column));
				gtk_table_attach_defaults (GTK_TABLE (my_table), GTK_WIDGET (my_position[row][column]), column, column+1, row, row+1);
				gtk_widget_show (GTK_WIDGET (my_position[row][column]));
			}
		}
	
		gtk_widget_destroy (his_table);
		his_table = gtk_table_new (NumberOfRows, NumberOfColumns, TRUE);
		gtk_box_pack_start (GTK_BOX (his_vbox), his_table, FALSE, FALSE, 0);
		gtk_widget_show (his_table);
	
		for (row=0;row<NumberOfRows;row++)
			for (column=0;column<NumberOfColumns;column++) {
				his_position[row][column] = GTK_BUTTON (gtk_button_new_with_label ("  "));
				his_image[row][column] = gtk_image_new_from_pixbuf (pixbuf_unknown);
				gtk_button_set_image (his_position[row][column], his_image[row][column]);
				his_handler_id[row][column] = g_signal_connect (G_OBJECT (his_position[row][column]), "button-press-event", G_CALLBACK (on_his_position_clicked), GINT_TO_POINTER (row*100+column));
				gtk_table_attach_defaults (GTK_TABLE (his_table), GTK_WIDGET (his_position[row][column]), column, column+1, row, row+1);
				gtk_widget_show (GTK_WIDGET (his_position[row][column]));
			}

		initialize ();
	}
	
	gtk_widget_destroy (preferences_window);
}


void preferences_changed (GtkAdjustment *adjustment, GtkWidget *pointer) {
	gint NumberOfRows, NumberOfColumns;
	gint NumberOfShips, MaximumNumberOfShips;
	gint DifficultyLimit;

	if (adjustment == NumberOfShipsAdjustment) {
		NumberOfShips = gtk_spin_button_get_value (ships_button);
		DifficultyLimit = gtk_spin_button_get_value (difficulty_button);
		
		DifficultyLimit = (DifficultyLimit<NumberOfShips ? DifficultyLimit : NumberOfShips-1);
		
		DifficultyAdjustment->value = DifficultyLimit;
		DifficultyAdjustment->upper = NumberOfShips-1;
	}
	else {
		NumberOfRows = gtk_spin_button_get_value (rows_button);
		NumberOfColumns = gtk_spin_button_get_value (columns_button);
		NumberOfShips = gtk_spin_button_get_value (ships_button);
		
		MaximumNumberOfShips = NumberOfRows + NumberOfColumns;
		NumberOfShips = (NumberOfShips<MaximumNumberOfShips ? NumberOfShips : MaximumNumberOfShips);

		NumberOfShipsAdjustment->value = NumberOfShips;
		NumberOfShipsAdjustment->upper = MaximumNumberOfShips;
		gtk_adjustment_value_changed (NumberOfShipsAdjustment);
	}
}


void menu_preferences () {
	GtkWidget *preferences_window, *table, *notebook, *label, *button;
	GtkWidget *hbox_difficulty, *hbox_help;
	GtkAdjustment *adjustment;
	gint MaximumNumberOfShips;
	
    preferences_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (GTK_WIDGET (preferences_window), -1, -1);
    gtk_window_set_title (GTK_WINDOW (preferences_window), _("Preferences"));
	gtk_window_set_position (GTK_WINDOW (preferences_window), GTK_WIN_POS_CENTER);
    //g_signal_connect (G_OBJECT (preferences_window), "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_signal_connect_swapped (G_OBJECT (preferences_window), "delete_event", G_CALLBACK (gtk_widget_destroy), G_OBJECT (preferences_window));

	table = gtk_table_new (3, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (preferences_window), table);
	gtk_widget_show (table);

    notebook = gtk_notebook_new ();
    gtk_table_attach_defaults (GTK_TABLE (table), notebook, 0, 3, 0, 1);
    gtk_widget_show (notebook);

	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (gtk_widget_destroy), G_OBJECT(preferences_window));
    gtk_table_attach_defaults (GTK_TABLE (table), button, 0, 1, 1, 2);
    gtk_widget_show (button);
	
	button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
    g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (preferences_closed), G_OBJECT(preferences_window));
    gtk_table_attach_defaults (GTK_TABLE (table), button, 2, 3, 1, 2);
    gtk_widget_show (button);
	
	table = gtk_table_new (2, 3, FALSE);
	gtk_widget_show (table);
	hbox_difficulty = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox_difficulty);
	hbox_help = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox_help);
	
	label = gtk_label_new (_("Size"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), table, label);
	label = gtk_label_new (_("Difficulty Level"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox_difficulty, label);
	label = gtk_label_new (_("Help Level"));
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox_help, label);

	label = gtk_label_new (_("Number of rows: "));
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);
	gtk_widget_show (label);
	adjustment = (GtkAdjustment *) gtk_adjustment_new (NumberOfRows, 2, 14, 1, 5, 0);
	rows_button = GTK_SPIN_BUTTON (gtk_spin_button_new (adjustment, 1, 0));
    g_signal_connect (G_OBJECT (adjustment), "value-changed", G_CALLBACK (preferences_changed), NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), GTK_WIDGET (rows_button), 1, 2, 0, 1);
	gtk_widget_show (GTK_WIDGET(rows_button));

	label = gtk_label_new (_("Number of columns: "));
    gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);
	gtk_widget_show (label);
	adjustment = (GtkAdjustment *) gtk_adjustment_new (NumberOfColumns, 2, 14, 1, 5, 0);
	columns_button = GTK_SPIN_BUTTON (gtk_spin_button_new (adjustment, 1, 0));
    g_signal_connect (G_OBJECT (adjustment), "value-changed", G_CALLBACK (preferences_changed), NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), GTK_WIDGET (columns_button), 1, 2, 1, 2);
	gtk_widget_show (GTK_WIDGET (columns_button));

	label = gtk_label_new (_("Number of ships: "));
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);
	gtk_widget_show (label);
	MaximumNumberOfShips = NumberOfRows + NumberOfColumns;
	NumberOfShipsAdjustment = (GtkAdjustment *) gtk_adjustment_new (NumberOfShips, 1, MaximumNumberOfShips, 1, 5, 0);
	ships_button = GTK_SPIN_BUTTON (gtk_spin_button_new (NumberOfShipsAdjustment, 1, 0));
    g_signal_connect (G_OBJECT (NumberOfShipsAdjustment), "value-changed", G_CALLBACK (preferences_changed), NULL);
    gtk_table_attach_defaults (GTK_TABLE (table), GTK_WIDGET (ships_button), 1, 2, 2, 3);
	gtk_widget_show (GTK_WIDGET (ships_button));

	label = gtk_label_new (_("Difficulty level: "));
	gtk_box_pack_start (GTK_BOX (hbox_difficulty), label, TRUE, FALSE, 0);
	gtk_widget_show (label);
	DifficultyAdjustment = (GtkAdjustment *) gtk_adjustment_new (DifficultyLimit, 1, NumberOfShips-1, 1, 5, 0);
	difficulty_button = GTK_SPIN_BUTTON (gtk_spin_button_new (DifficultyAdjustment, 1, 0));
	gtk_widget_set_tooltip_text (GTK_WIDGET (difficulty_button), 
								 _("The lower the difficulty level the higher the probability that "
								   "only a single enemy ship is put to each row and each column."));
	gtk_box_pack_start (GTK_BOX (hbox_difficulty), GTK_WIDGET (difficulty_button), TRUE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET (difficulty_button));

	label = gtk_label_new (_("Help level: "));
	gtk_box_pack_start (GTK_BOX (hbox_help), label, TRUE, FALSE, 0);
	gtk_widget_show (label);
	adjustment = (GtkAdjustment *) gtk_adjustment_new (help, 0, 3, 1, 1, 0);
	help_button = GTK_SPIN_BUTTON (gtk_spin_button_new (adjustment, 1, 0));
	gtk_widget_set_tooltip_text (GTK_WIDGET (help_button),
								 _("The higher the help level the more obviously unoccupied positions "
								   "are marked as unoccupied automatically."));
	gtk_box_pack_start (GTK_BOX (hbox_help), GTK_WIDGET (help_button), TRUE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET (help_button));

    gtk_widget_show (preferences_window);
}


void menu_help () {
	GtkWidget *help;
	
	help = gtk_message_dialog_new (GTK_WINDOW (window), 
								   GTK_DIALOG_DESTROY_WITH_PARENT, 
								   GTK_MESSAGE_INFO,
								   GTK_BUTTONS_OK,
								   _("To start place your ships on the left panel.\n\n"
								   "Fire with left mouse button on enemy ships on the right panel.\n"
								   "The displayed number tells you the total number\n"
								   "of ships in this row and this column.\n\n"
								   "You can mark empty positions with right mouse button.\n"));
	gtk_dialog_run (GTK_DIALOG (help));
	gtk_widget_destroy (help);
}


void menu_about () {
	extern GdkPixbuf *pixbuf_logo;
	gchar *authors[] = {"Bruno Ramalhete (bram.512@gmail.com)", NULL};
	gchar *documenters[] = {"Bruno Ramalhete (bram.512@gmail.com)", NULL};
	gchar *artists[] = {"Bruno Ramalhete (thomas.finteis@gmx.d)", NULL};
	gtk_show_about_dialog (NULL,
						   "authors", authors,
						   "documenters", documenters,
						   /*TRANSLATORS: Please put here your name and email address in
							parantheses in order to appear in the translator credits. */
						   "translator-credits", _("Thomas Finteis (thomas.finteis@gmx.de)\nBruno Ramalhete (bram.512@gmail.com)"),
						   "artists", artists,
						   "comments", _("Battle Ship.\n"
										 "Suggestions for improvements, bug reports and translations are welcome."),
						   "program-name", PACKAGE,
						   "version", VERSION,
						   "copyright", "© 2015-2016 Bruno Ramalhete",
						   "license", 
						   _("This program is free software; you can redistribute it and/or modify "
						   "it under the terms of the GNU General Public License as published by "
						   "the Free Software Foundation; either version 2 of the License, or "
						   "(at your option) any later version."
						   "\n\n" 
						   "This program is distributed in the hope that it will be useful, "
						   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
						   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
						   "GNU Library General Public License for more details."
						   "\n\n" 
						   "You should have received a copy of the GNU General Public License "
						   "along with this program; if not, see http://www.gnu.org/licenses."),
						   "wrap-license", TRUE,
						   "logo", pixbuf_logo,
						   "website", "https://sourceforge.net/bruno256",
						   NULL);
}


void menu_exit () {
	extern GtkWidget *window;
	
	gtk_widget_destroy (window);
}
