/* treeview-hildon.c: autotest the different Hildon GtkTreeView modes.
 *
 * Copyright (C) 2008  Nokia Corporation
 * Author: Kristian Rietveld <kris@imendio.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>

/* Fixture */
typedef struct
{
  GtkWidget *window;
  GtkWidget *tree_view;
  GtkTreeModel *model;
  GtkTreeSelection *selection;
}
HildonTreeViewFixture;

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
hildon_tree_view_fixture_setup (HildonTreeViewFixture *fixture,
                                gboolean               multi,
                                gboolean               edit,
                                gconstpointer          test_data)
{
  HildonUIMode mode;
  GtkCellRenderer *renderer;

  fixture->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  if (edit)
    mode = HILDON_UI_MODE_EDIT;
  else
    mode = HILDON_UI_MODE_NORMAL;

  fixture->model = create_model ();
  fixture->tree_view = g_object_new (GTK_TYPE_TREE_VIEW,
                                     "model", fixture->model,
                                     "name", "fremantle-widget",
                                     "hildon-ui-mode", mode,
                                     "rules-hint", TRUE,
                                     NULL);
  g_object_unref (fixture->model);

  fixture->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (fixture->tree_view));
  if (multi)
    gtk_tree_selection_set_mode (fixture->selection, GTK_SELECTION_MULTIPLE);
  else if (mode != HILDON_UI_MODE_NORMAL)
    gtk_tree_selection_set_mode (fixture->selection, GTK_SELECTION_SINGLE);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (renderer,
                "xalign", 0.5,
                "weight", PANGO_WEIGHT_BOLD,
                NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (fixture->tree_view),
                                               0, "Column 0",
                                               renderer,
                                               "text", 0,
                                               NULL);

  gtk_container_add (GTK_CONTAINER (fixture->window), fixture->tree_view);
  gtk_widget_show_all (fixture->window);
}

static void
hildon_tree_view_fixture_single_setup (HildonTreeViewFixture *fixture,
                                       gconstpointer          test_data)
{
  hildon_tree_view_fixture_setup (fixture, FALSE, FALSE, test_data);
}

static void
hildon_tree_view_fixture_multi_setup (HildonTreeViewFixture *fixture,
                                      gconstpointer          test_data)
{
  hildon_tree_view_fixture_setup (fixture, TRUE, FALSE, test_data);
}

static void
hildon_tree_view_fixture_edit_single_setup (HildonTreeViewFixture *fixture,
                                            gconstpointer          test_data)
{
  hildon_tree_view_fixture_setup (fixture, FALSE, TRUE, test_data);
}

static void
hildon_tree_view_fixture_edit_multi_setup (HildonTreeViewFixture *fixture,
                                           gconstpointer          test_data)
{
  hildon_tree_view_fixture_setup (fixture, TRUE, TRUE, test_data);
}


static void
hildon_tree_view_fixture_teardown (HildonTreeViewFixture *fixture,
                                   gconstpointer          test_data)
{
  gtk_widget_destroy (fixture->window);
}


/* Tests */
static void
normal_selection_none (HildonTreeViewFixture *fixture,
                       gconstpointer          test_data)
{
  /* Verify the selection mode is forced to be NONE */
  g_assert (gtk_tree_selection_get_mode (fixture->selection) == GTK_SELECTION_NONE);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 0);
}

static void
edit_selection_single (HildonTreeViewFixture *fixture,
                       gconstpointer          test_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;

  /* One item must be selected */
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);

  /* Selection should move */
  path = gtk_tree_path_new_from_indices (10, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));

  /* When selected item is deleted, first item should get selection */
  gtk_tree_model_get_iter (fixture->model, &iter, path);
  gtk_tree_path_free (path);

  gtk_list_store_remove (GTK_LIST_STORE (fixture->model), &iter);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);

  path = gtk_tree_path_new_from_indices (0, -1);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));
  gtk_tree_path_free (path);
}

static void
edit_selection_multi (HildonTreeViewFixture *fixture,
                      gconstpointer          test_data)
{
  GtkTreePath *path;
  GtkTreeIter iter;

  /* Selection must be empty */
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 0);

  /* Selection should be extended */
  path = gtk_tree_path_new_from_indices (10, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));

  /* When selected item is deleted, the selection will be empty */
  gtk_tree_model_get_iter (fixture->model, &iter, path);
  gtk_tree_path_free (path);

  gtk_list_store_remove (GTK_LIST_STORE (fixture->model), &iter);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 0);

  /* Selection should be extended */
  path = gtk_tree_path_new_from_indices (10, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));
  gtk_tree_path_free (path);

  /* Selection should be extended */
  path = gtk_tree_path_new_from_indices (20, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 2);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));
  gtk_tree_path_free (path);
}

static void
edit_multi_to_single (HildonTreeViewFixture *fixture,
                      gconstpointer          test_data)
{
  GtkTreePath *path;

  /* Selection must be empty */
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 0);

  /* Select and unselect a row */
  path = gtk_tree_path_new_from_indices (10, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));

  gtk_tree_selection_unselect_path (fixture->selection, path);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path) != TRUE);
  gtk_tree_path_free (path);

  /* Select two rows */
  path = gtk_tree_path_new_from_indices (10, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));
  gtk_tree_path_free (path);

  path = gtk_tree_path_new_from_indices (20, -1);
  gtk_tree_selection_select_path (fixture->selection, path);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 2);
  g_assert (gtk_tree_selection_path_is_selected (fixture->selection, path));
  gtk_tree_path_free (path);

  /* Switch selection mode, one item should stay selected */
  gtk_tree_selection_set_mode (fixture->selection, GTK_SELECTION_SINGLE);
  g_assert (gtk_tree_selection_count_selected_rows (fixture->selection) == 1);
}

static void
normal_to_edit_test (HildonTreeViewFixture *fixture,
                     gconstpointer          test_data)
{
  normal_selection_none (fixture, test_data);

  g_object_set (fixture->tree_view,
                "hildon-ui-mode", HILDON_UI_MODE_EDIT,
                NULL);

  edit_selection_single (fixture, test_data);
}

static void
edit_multi_to_normal_test (HildonTreeViewFixture *fixture,
                           gconstpointer          test_data)
{
  edit_selection_multi (fixture, test_data);

  g_object_set (fixture->tree_view,
                "hildon-ui-mode", HILDON_UI_MODE_NORMAL,
                NULL);

  normal_selection_none (fixture, test_data);
}

int
main (int argc, char **argv)
{
  gtk_test_init (&argc, &argv);

  gtk_rc_parse_string ("style \"fremantle-widget\" {\n"
                       "  GtkWidget::hildon-mode = 1\n" 
                       "} widget \"*.fremantle-widget\" style \"fremantle-widget\"");

  g_test_add ("/treeview/hildon/normal-single-selection-none",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_single_setup,
              normal_selection_none,
              hildon_tree_view_fixture_teardown);
  g_test_add ("/treeview/hildon/normal-multi-selection-none",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_multi_setup,
              normal_selection_none,
              hildon_tree_view_fixture_teardown);

  g_test_add ("/treeview/hildon/edit-single-selection-test",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_edit_single_setup,
              edit_selection_single,
              hildon_tree_view_fixture_teardown);
  g_test_add ("/treeview/hildon/edit-multi-selection-test",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_edit_multi_setup,
              edit_selection_multi,
              hildon_tree_view_fixture_teardown);

  g_test_add ("/treeview/hildon/edit-multi-to-single-test",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_edit_multi_setup,
              edit_multi_to_single,
              hildon_tree_view_fixture_teardown);

  g_test_add ("/treeview/hildon/normal-to-edit-test",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_single_setup,
              normal_to_edit_test,
              hildon_tree_view_fixture_teardown);
  g_test_add ("/treeview/hildon/edit-multi-to-normal-test",
              HildonTreeViewFixture, NULL,
              hildon_tree_view_fixture_edit_multi_setup,
              edit_multi_to_normal_test,
              hildon_tree_view_fixture_teardown);

  return g_test_run ();
}
