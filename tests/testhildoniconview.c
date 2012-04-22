/* testhildoniconview.c: test the different Hildon GtkIconView modes.
 *
 * Copyright (C) 2008  Nokia Corporation
 * Author: Kristian Rietveld <kris@imendio.com>
 *
 * This work is provided "as is"; redistribution and modification
 * in whole or in part, in any medium, physical or electronic is
 * permitted without restriction.
 *
 * This work is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * In no event shall the authors or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 */

#include <gtk/gtk.h>

enum
{
  MULTI_SELECT  = 1 << 0,
  NORMAL_MODE   = 1 << 1,
  EDIT_MODE     = 1 << 2
};


static GtkTreeModel *
create_model (void)
{
  int i;
  GtkListStore *store;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  for (i = 0; i < 50; i++)
    {
      gchar *str;

      str = g_strdup_printf ("\nRow %d\n", i);
      gtk_list_store_insert_with_values (store, NULL, i, 0, str, -1);
      g_free (str);
    }

  return GTK_TREE_MODEL (store);
}

static void
item_activated_callback (GtkWidget         *icon_view,
                         GtkTreePath       *path,
                         gpointer           user_data)
{
  g_print ("item-activated emitted.\n");
}

static gboolean
row_header_func (GtkTreeModel  *model,
                 GtkTreeIter   *iter,
                 gchar        **header_text,
                 gpointer       data)
{
  GtkTreePath *path;
  gboolean retval = FALSE;

  path = gtk_tree_model_get_path (model, iter);
  if (gtk_tree_path_get_indices (path)[0] % 15 == 0)
    {
      retval = TRUE;
      if (header_text)
        *header_text = g_strdup ("Happy fun group header!");
    }
  gtk_tree_path_free (path);

  return retval;
}

static GtkWidget *
create_icon_view (HildonUIMode  mode,
                  const char   *name,
                  gboolean      multi_select)
{
  GtkWidget *icon_view;
  GtkCellRenderer *renderer;

  icon_view = g_object_new (GTK_TYPE_ICON_VIEW,
                            "model", create_model (),
                            "name", name,
                            "hildon-ui-mode", mode,
                            NULL);

  if (multi_select)
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                      GTK_SELECTION_MULTIPLE);
  else if (mode != HILDON_UI_MODE_NORMAL)
    gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                      GTK_SELECTION_SINGLE);

  hildon_icon_view_set_row_header_func (GTK_ICON_VIEW (icon_view),
                                        row_header_func,
                                        NULL,
                                        NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "stock-id", GTK_STOCK_NEW, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (icon_view),
                              renderer,
                              TRUE);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (icon_view),
                              renderer,
                              FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (icon_view),
                                  renderer,
                                  "text", 0,
                                  NULL);

  return icon_view;
}

static void
create_icon_view_window (GtkWidget *button,
                         gpointer   user_data)
{
  const char *name;
  GtkWidget *window;
  GtkWidget *sw;
  GtkWidget *icon_view;
  HildonUIMode mode = 0;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_widget_destroy), window);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (window), sw);

  if ((GPOINTER_TO_INT (user_data) & NORMAL_MODE) == NORMAL_MODE)
    {
      mode = HILDON_UI_MODE_NORMAL;
      name = "fremantle-widget";
    }
  else if ((GPOINTER_TO_INT (user_data) & EDIT_MODE) == EDIT_MODE)
    {
      mode = HILDON_UI_MODE_EDIT;
      name = "fremantle-widget";
    }
  else
    name = NULL;

  icon_view = create_icon_view (mode, name,
                                (GPOINTER_TO_INT (user_data) & MULTI_SELECT) == MULTI_SELECT);

  /* Some signals doing printfs() to see if the behavior is correct. */
  g_signal_connect (icon_view, "item-activated",
                    G_CALLBACK (item_activated_callback), NULL);

  gtk_widget_set_size_request (icon_view, 480, 800);
  gtk_container_add (GTK_CONTAINER (sw), icon_view);

  gtk_widget_show_all (window);
}

int
main (int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *mainbox;
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *padbox;
  GtkWidget *button;

  gtk_init (&argc, &argv);

  gtk_rc_parse_string ("style \"fremantle-widget\" {\n"
                       "  GtkWidget::hildon-mode = 1\n"
                       "} widget \"*.fremantle-widget\" style \"fremantle-widget\"");

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  mainbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window), mainbox);

  /* old-style */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Old-style behavior</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("   "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Single selection");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window), 0x0);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Multiple selections");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (MULTI_SELECT));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  /* normal-mode */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Normal mode</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("   "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Direct activation");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (NORMAL_MODE));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  /* edit-mode */
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Edit mode</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("   "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 6);

  button = gtk_button_new_with_label ("Single selection");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (EDIT_MODE));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_button_new_with_label ("Multiple selections");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (create_icon_view_window),
                    GINT_TO_POINTER (EDIT_MODE | MULTI_SELECT));
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);


  button = gtk_button_new_with_label ("Close");
  g_signal_connect (button, "clicked",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_box_pack_end (GTK_BOX (mainbox), button, FALSE, FALSE, 0);

  gtk_box_pack_end (GTK_BOX (mainbox), gtk_hseparator_new (),
                    FALSE, FALSE, 6);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
