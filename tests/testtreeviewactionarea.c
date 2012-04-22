/* testtreeviewactionarea.c: test action area support in GtkTreeView.
 *
 * Copyright: (C) 2009  Nokia Corporation.
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

static GtkTreeModel *
create_model (void)
{
  int i;
  GtkListStore *store;

  store = gtk_list_store_new (1, G_TYPE_STRING);

  for (i = 0; i < 50; i++)
    {
      gchar *str;

      str = g_strdup_printf ("Row %d", i);
      gtk_list_store_insert_with_values (store, NULL, i, 0, str, -1);
      g_free (str);
    }

  return GTK_TREE_MODEL (store);
}

static void
portrait_toggled (GtkToggleButton *button,
                  gpointer         user_data)
{
  if (gtk_toggle_button_get_active (button))
    {
      GtkWidget *tree_view;

      gtk_window_resize (GTK_WINDOW (user_data), 480, 800);

      tree_view = gtk_bin_get_child (GTK_BIN (user_data));
      tree_view = gtk_bin_get_child (GTK_BIN (tree_view));

      hildon_tree_view_set_action_area_orientation (GTK_TREE_VIEW (tree_view),
                                                    GTK_ORIENTATION_VERTICAL);
    }
}

static void
landscape_toggled (GtkToggleButton *button,
                   gpointer         user_data)
{
  if (gtk_toggle_button_get_active (button))
    {
      GtkWidget *tree_view;

      gtk_window_resize (GTK_WINDOW (user_data), 800, 480);

      tree_view = gtk_bin_get_child (GTK_BIN (user_data));
      tree_view = gtk_bin_get_child (GTK_BIN (tree_view));

      hildon_tree_view_set_action_area_orientation (GTK_TREE_VIEW (tree_view),
                                                    GTK_ORIENTATION_HORIZONTAL);
    }
}

static void
show_buttons_toggled (GtkToggleButton *toggle_button,
                      gpointer         user_data)
{
  gboolean value;
  GtkTreeView *tree_view = GTK_TREE_VIEW (user_data);

  value = hildon_tree_view_get_action_area_visible (tree_view);
  value = !value;

  hildon_tree_view_set_action_area_visible (tree_view, value);
}

static gboolean
row_header_func (GtkTreeModel *model,
                 GtkTreeIter  *iter,
                 gchar       **header_text,
                 gpointer      user_data)
{
  GtkTreePath *path;
  gboolean retval = FALSE;

  path = gtk_tree_model_get_path (model, iter);
  if (gtk_tree_path_get_indices (path)[0] % 10 == 0)
    {
      retval = TRUE;
      if (header_text)
        *header_text = g_strdup ("Happy fun group header!");
    }
  gtk_tree_path_free (path);

  return retval;
}

static GtkWidget *
create_tree_view (void)
{
  GtkWidget *tree_view;
  GtkCellRenderer *renderer;

  tree_view = g_object_new (GTK_TYPE_TREE_VIEW,
                            "model", create_model (),
                            "name", "fremantle-widget",
                            "hildon-ui-mode", HILDON_UI_MODE_NORMAL,
                            "rules-hint", TRUE,
                            NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "xalign", 0.5,
                "weight", PANGO_WEIGHT_BOLD,
                NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (tree_view),
                                               0, "Column 0",
                                               renderer,
                                               "text", 0,
                                               NULL);

  hildon_tree_view_set_row_header_func (GTK_TREE_VIEW (tree_view),
                                        row_header_func,
                                        NULL,
                                        NULL);

  return tree_view;

}


int
main (int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *tree_window;
  GtkWidget *mainbox;
  GtkWidget *padbox;
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *button;
  GtkWidget *check_button;
  GtkWidget *tree_view;
  GtkWidget *scrolled;

  gtk_init (&argc, &argv);

  gtk_rc_parse_string ("style \"fremantle-widget\" {\n"
                       "  GtkWidget::hildon-mode = 1\n"
                       "  GtkTreeView::row-height = 70\n"
                       "} widget \"*.fremantle-widget\" style \"fremantle-widget\"");

  /* Tree view window */
  tree_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (tree_window, "delete-event",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (tree_window), 6);

  scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (tree_window), scrolled);

  tree_view = create_tree_view ();
  gtk_container_add (GTK_CONTAINER (scrolled), tree_view);

  vbox = hildon_tree_view_get_action_area_box (GTK_TREE_VIEW (tree_view));
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  button = gtk_button_new_with_label ("Button 1");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("Button 2");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0);

  gtk_window_resize (GTK_WINDOW (tree_window), 800, 480);

  gtk_widget_show_all (tree_window);

  /* Control window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "delete-event",
                    G_CALLBACK (gtk_main_quit), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (window), 6);

  mainbox = gtk_vbox_new (FALSE, 6);
  gtk_container_add (GTK_CONTAINER (window), mainbox);

  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), "<b>Orientation</b>");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (mainbox), label, FALSE, FALSE, 0);

  padbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (mainbox), padbox, FALSE, FALSE, 6);

  gtk_box_pack_start (GTK_BOX (padbox), gtk_label_new ("  "),
                      FALSE, FALSE, 6);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (padbox), vbox, TRUE, TRUE, 0);

  button = gtk_radio_button_new_with_label (NULL, "Landscape");
  g_signal_connect (button, "toggled",
                    G_CALLBACK (landscape_toggled), tree_window);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

  button = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (button), "Portrait");
  g_signal_connect (button, "toggled",
                    G_CALLBACK (portrait_toggled), tree_window);
  gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);


  check_button = gtk_check_button_new_with_label ("Show buttons");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button), FALSE);
  g_signal_connect (check_button, "toggled",
                    G_CALLBACK (show_buttons_toggled), tree_view);
  gtk_box_pack_start (GTK_BOX (mainbox), check_button, FALSE, FALSE, 0);


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
