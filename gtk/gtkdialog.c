/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#undef GTK_DISABLE_DEPRECATED

#include <stdlib.h>
#include <string.h>
#include "config.h"
#ifdef MAEMO_CHANGES
#include "gtkalignment.h"
#endif /* MAEMO_CHANGES */
#include "gtkbutton.h"
#include "gtkdialog.h"
#include "gtkhbbox.h"
#include "gtkvbbox.h"
#include "gtklabel.h"
#include "gtkhseparator.h"
#include "gtkvseparator.h"
#include "gtkmarshalers.h"
#include "gtkhbox.h"
#include "gtkvbox.h"
#include "gdkkeysyms.h"
#include "gtkmain.h"
#include "gtkintl.h"
#include "gtkbindings.h"
#include "gtkprivate.h"
#include "gtkbuildable.h"
#include "gtkalias.h"

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_DIALOG, GtkDialogPrivate))

typedef struct {
  guint ignore_separator : 1;
#ifdef MAEMO_CHANGES
  GtkWidget *hbox;
  GtkWidget *alignment;
#endif /* MAEMO_CHANGES */
} GtkDialogPrivate;

typedef struct _ResponseData ResponseData;

struct _ResponseData
{
  gint response_id;
};

static void      gtk_dialog_add_buttons_valist   (GtkDialog    *dialog,
                                                  const gchar  *first_button_text,
                                                  va_list       args);

static gboolean  gtk_dialog_delete_event_handler (GtkWidget    *widget,
                                                  GdkEventAny  *event,
                                                  gpointer      user_data);

static void      gtk_dialog_set_property         (GObject      *object,
                                                  guint         prop_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec);
static void      gtk_dialog_get_property         (GObject      *object,
                                                  guint         prop_id,
                                                  GValue       *value,
                                                  GParamSpec   *pspec);
static void      gtk_dialog_style_set            (GtkWidget    *widget,
                                                  GtkStyle     *prev_style);
#ifdef MAEMO_CHANGES
static void      gtk_dialog_size_request         (GtkWidget      *widget,
                                                  GtkRequisition *requisition);
static void      gtk_dialog_realize              (GtkWidget    *widget);
static void      gtk_dialog_unrealize            (GtkWidget    *widget);
#endif
static void      gtk_dialog_map                  (GtkWidget    *widget);

static void      gtk_dialog_close                (GtkDialog    *dialog);

static ResponseData * get_response_data          (GtkWidget    *widget,
                                                  gboolean      create);

static void      gtk_dialog_buildable_interface_init     (GtkBuildableIface *iface);
static GObject * gtk_dialog_buildable_get_internal_child (GtkBuildable  *buildable,
                                                          GtkBuilder    *builder,
                                                          const gchar   *childname);
static gboolean  gtk_dialog_buildable_custom_tag_start   (GtkBuildable  *buildable,
                                                          GtkBuilder    *builder,
                                                          GObject       *child,
                                                          const gchar   *tagname,
                                                          GMarkupParser *parser,
                                                          gpointer      *data);
static void      gtk_dialog_buildable_custom_finished    (GtkBuildable  *buildable,
                                                          GtkBuilder    *builder,
                                                          GObject       *child,
                                                          const gchar   *tagname,
                                                          gpointer       user_data);


enum {
  PROP_0,
#ifdef MAEMO_CHANGES
  PROP_TOP_PADDING,
  PROP_BOTTOM_PADDING,
  PROP_LEFT_PADDING,
  PROP_RIGHT_PADDING,
  PROP_INNER_SPACING,
#endif /* MAEMO_CHANGES */
  PROP_HAS_SEPARATOR
};

enum {
  RESPONSE,
  CLOSE,
  LAST_SIGNAL
};

static guint dialog_signals[LAST_SIGNAL];

G_DEFINE_TYPE_WITH_CODE (GtkDialog, gtk_dialog, GTK_TYPE_WINDOW,
			 G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE,
						gtk_dialog_buildable_interface_init))

static void
gtk_dialog_class_init (GtkDialogClass *class)
{
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;
  GtkBindingSet *binding_set;
  
  gobject_class = G_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);
  
  gobject_class->set_property = gtk_dialog_set_property;
  gobject_class->get_property = gtk_dialog_get_property;
  
  widget_class->map = gtk_dialog_map;
  widget_class->style_set = gtk_dialog_style_set;
#ifdef MAEMO_CHANGES
  widget_class->size_request = gtk_dialog_size_request;
  widget_class->realize = gtk_dialog_realize;
  widget_class->unrealize = gtk_dialog_unrealize;
#endif

  class->close = gtk_dialog_close;
  
  g_type_class_add_private (gobject_class, sizeof (GtkDialogPrivate));

  /**
   * GtkDialog:has-separator:
   *
   * When %TRUE, the dialog has a separator bar above its buttons.
   *
   * Deprecated: 2.22: This property will be removed in GTK+ 3.
   */
  g_object_class_install_property (gobject_class,
                                   PROP_HAS_SEPARATOR,
                                   g_param_spec_boolean ("has-separator",
							 P_("Has separator"),
							 P_("The dialog has a separator bar above its buttons"),
                                                         FALSE,
                                                         GTK_PARAM_READWRITE | G_PARAM_DEPRECATED));

#ifdef MAEMO_CHANGES
  g_object_class_install_property (gobject_class,
                                   PROP_TOP_PADDING,
                                   g_param_spec_uint ("top-padding",
                                                      P_("Top Padding"),
                                                      P_("The padding to insert at the top of the dialog."),
                                                      0,
                                                      G_MAXUINT,
                                                      0,
                                                      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class,
                                   PROP_BOTTOM_PADDING,
                                   g_param_spec_uint ("bottom-padding",
                                                      P_("Bottom Padding"),
                                                      P_("The padding to insert at the bottom of the dialog."),
                                                      0,
                                                      G_MAXUINT,
                                                      8,
                                                      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class,
                                   PROP_LEFT_PADDING,
                                   g_param_spec_uint ("left-padding",
                                                      P_("Left Padding"),
                                                      P_("The padding to insert at the left of the dialog."),
                                                      0,
                                                      G_MAXUINT,
                                                      16,
                                                      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class,
                                   PROP_RIGHT_PADDING,
                                   g_param_spec_uint ("right-padding",
                                                      P_("Right Padding"),
                                                      P_("The padding to insert at the right of the dialog."),
                                                      0,
                                                      G_MAXUINT,
                                                      16,
                                                      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_object_class_install_property (gobject_class,
                                   PROP_INNER_SPACING,
                                   g_param_spec_int ("inner-spacing",
                                                      P_("Inner Spacing"),
                                                      P_("The spacing between content area and action area."),
                                                      0,
                                                      G_MAXINT,
                                                      16,
                                                      GTK_PARAM_READWRITE | G_PARAM_CONSTRUCT));
#endif /* MAEMO_CHANGES */

  /**
   * GtkDialog::response:
   * @dialog: the object on which the signal is emitted
   * @response_id: the response ID
   * 
   * Emitted when an action widget is clicked, the dialog receives a 
   * delete event, or the application programmer calls gtk_dialog_response(). 
   * On a delete event, the response ID is #GTK_RESPONSE_DELETE_EVENT. 
   * Otherwise, it depends on which action widget was clicked.
   */
  dialog_signals[RESPONSE] =
    g_signal_new (I_("response"),
		  G_OBJECT_CLASS_TYPE (class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GtkDialogClass, response),
		  NULL, NULL,
		  _gtk_marshal_VOID__INT,
		  G_TYPE_NONE, 1,
		  G_TYPE_INT);

  /**
   * GtkDialog::close:
   *
   * The ::close signal is a 
   * <link linkend="keybinding-signals">keybinding signal</link>
   * which gets emitted when the user uses a keybinding to close
   * the dialog.
   *
   * The default binding for this signal is the Escape key.
   */ 
  dialog_signals[CLOSE] =
    g_signal_new (I_("close"),
		  G_OBJECT_CLASS_TYPE (class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (GtkDialogClass, close),
		  NULL, NULL,
		  _gtk_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);
  
  gtk_widget_class_install_style_property (widget_class,
					   g_param_spec_int ("content-area-border",
                                                             P_("Content area border"),
                                                             P_("Width of border around the main dialog area"),
                                                             0,
                                                             G_MAXINT,
                                                             2,
                                                             GTK_PARAM_READABLE));
  /**
   * GtkDialog:content-area-spacing:
   *
   * The default spacing used between elements of the
   * content area of the dialog, as returned by
   * gtk_dialog_get_content_area(), unless gtk_box_set_spacing()
   * was called on that widget directly.
   *
   * Since: 2.16
   */
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("content-area-spacing",
                                                             P_("Content area spacing"),
                                                             P_("Spacing between elements of the main dialog area"),
                                                             0,
                                                             G_MAXINT,
                                                             0,
                                                             GTK_PARAM_READABLE));
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("button-spacing",
                                                             P_("Button spacing"),
                                                             P_("Spacing between buttons"),
                                                             0,
                                                             G_MAXINT,
                                                             6,
                                                             GTK_PARAM_READABLE));
  
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("action-area-border",
                                                             P_("Action area border"),
                                                             P_("Width of border around the button area at the bottom of the dialog"),
                                                             0,
                                                             G_MAXINT,
                                                             5,
                                                             GTK_PARAM_READABLE));

  binding_set = gtk_binding_set_by_class (class);
  
  gtk_binding_entry_add_signal (binding_set, GDK_Escape, 0, "close", 0);
}

static void
update_spacings (GtkDialog *dialog)
{
  gint content_area_border;
  gint content_area_spacing;
  gint button_spacing;
  gint action_area_border;
#ifdef MAEMO_CHANGES
  GtkDialogPrivate *priv;
  guint top_padding;
  guint bottom_padding;
  guint left_padding;
  guint right_padding;

  priv = GET_PRIVATE (dialog);
#endif /* MAEMO_CHANGES */

  gtk_widget_style_get (GTK_WIDGET (dialog),
                        "content-area-border", &content_area_border,
                        "content-area-spacing", &content_area_spacing,
                        "button-spacing", &button_spacing,
                        "action-area-border", &action_area_border,
                        NULL);

#ifdef MAEMO_CHANGES
  gtk_dialog_get_padding (dialog, &top_padding, &bottom_padding, &left_padding, &right_padding);

  gtk_alignment_set_padding (GTK_ALIGNMENT (priv->alignment),
                             top_padding,
                             bottom_padding,
                             left_padding,
                             right_padding);

  gtk_box_set_spacing (GTK_BOX (priv->hbox), gtk_dialog_get_inner_spacing (dialog));
#endif /* MAEMO_CHANGES */
  gtk_container_set_border_width (GTK_CONTAINER (dialog->vbox),
                                  content_area_border);
  if (!_gtk_box_get_spacing_set (GTK_BOX (dialog->vbox)))
    {
      gtk_box_set_spacing (GTK_BOX (dialog->vbox), content_area_spacing);
      _gtk_box_set_spacing_set (GTK_BOX (dialog->vbox), FALSE);
    }
  gtk_box_set_spacing (GTK_BOX (dialog->action_area),
                       button_spacing);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->action_area),
                                  action_area_border);
}

static void
gtk_dialog_init (GtkDialog *dialog)
{
  GtkDialogPrivate *priv;

  priv = GET_PRIVATE (dialog);
  priv->ignore_separator = FALSE;

#ifdef MAEMO_CHANGES
  priv->hbox = gtk_hbox_new (FALSE, 0);
  priv->alignment = gtk_alignment_new (0.0, 0.0, 1.0, 1.0);
#endif /* MAEMO_CHANGES */

  /* To avoid breaking old code that prevents destroy on delete event
   * by connecting a handler, we have to have the FIRST signal
   * connection on the dialog.
   */
  g_signal_connect (dialog,
                    "delete-event",
                    G_CALLBACK (gtk_dialog_delete_event_handler),
                    NULL);

  dialog->vbox = gtk_vbox_new (FALSE, 0);

#ifdef MAEMO_CHANGES
  gtk_container_add (GTK_CONTAINER (priv->alignment), priv->hbox);
  gtk_container_add (GTK_CONTAINER (dialog), priv->alignment);
  gtk_widget_show (priv->hbox);
  gtk_widget_show (priv->alignment);
  gtk_widget_show (dialog->vbox);

  dialog->action_area = gtk_vbutton_box_new ();
  gtk_widget_set_name (dialog->action_area, "hildon-dialog-action-area");
#else
  gtk_container_add (GTK_CONTAINER (dialog), dialog->vbox);
  gtk_widget_show (dialog->vbox);

  dialog->action_area = gtk_hbutton_box_new ();
#endif /* MAEMO_CHANGES */

  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog->action_area),
                             GTK_BUTTONBOX_END);

#ifdef MAEMO_CHANGES
  gtk_box_pack_end (GTK_BOX (priv->hbox), dialog->action_area,
                    FALSE, TRUE, 0);
  gtk_widget_show (dialog->action_area);

  gtk_box_pack_end (GTK_BOX (priv->hbox), dialog->vbox, TRUE, TRUE, 0);
#else
  gtk_box_pack_end (GTK_BOX (dialog->vbox), dialog->action_area,
                    FALSE, TRUE, 0);
  gtk_widget_show (dialog->action_area);

  dialog->separator = NULL;
#endif /* MAEMO_CHANGES */

  gtk_window_set_type_hint (GTK_WINDOW (dialog),
                            GDK_WINDOW_TYPE_HINT_DIALOG);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
}

static GtkBuildableIface *parent_buildable_iface;

static void
gtk_dialog_buildable_interface_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);
  iface->get_internal_child = gtk_dialog_buildable_get_internal_child;
  iface->custom_tag_start = gtk_dialog_buildable_custom_tag_start;
  iface->custom_finished = gtk_dialog_buildable_custom_finished;
}

static GObject *
gtk_dialog_buildable_get_internal_child (GtkBuildable *buildable,
					 GtkBuilder   *builder,
					 const gchar  *childname)
{
    if (strcmp (childname, "vbox") == 0)
      return G_OBJECT (GTK_DIALOG (buildable)->vbox);
    else if (strcmp (childname, "action_area") == 0)
      return G_OBJECT (GTK_DIALOG (buildable)->action_area);

    return parent_buildable_iface->get_internal_child (buildable,
						       builder,
						       childname);
}

static void 
gtk_dialog_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  GtkDialog *dialog;
#ifdef MAEMO_CHANGES
  guint padding_top;
  guint padding_bottom;
  guint padding_left;
  guint padding_right;
#endif
  
  dialog = GTK_DIALOG (object);

#ifdef MAEMO_CHANGES
  gtk_dialog_get_padding (dialog, &padding_top, &padding_bottom, &padding_left, &padding_right);
#endif
  
  switch (prop_id)
    {
    case PROP_HAS_SEPARATOR:
      gtk_dialog_set_has_separator (dialog, g_value_get_boolean (value));
      break;
#ifdef MAEMO_CHANGES
    case PROP_TOP_PADDING:
      gtk_dialog_set_padding (dialog,
                              g_value_get_uint (value),
                              padding_bottom,
                              padding_left,
                              padding_right);
      break;
    case PROP_BOTTOM_PADDING:
      gtk_dialog_set_padding (dialog,
                              padding_top,
                              g_value_get_uint (value),
                              padding_left,
                              padding_right);
      break;
    case PROP_LEFT_PADDING:
      gtk_dialog_set_padding (dialog,
                              padding_top,
                              padding_bottom,
                              g_value_get_uint (value),
                              padding_right);
      break;
    case PROP_RIGHT_PADDING:
      gtk_dialog_set_padding (dialog,
                              padding_top,
                              padding_bottom,
                              padding_left,
                              g_value_get_uint (value));
      break;
    case PROP_INNER_SPACING:
      gtk_dialog_set_inner_spacing (dialog, g_value_get_int (value));
      break;
#endif /* MAEMO_CHANGES */

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void 
gtk_dialog_get_property (GObject     *object,
                         guint        prop_id,
                         GValue      *value,
                         GParamSpec  *pspec)
{
  GtkDialog *dialog;
#ifdef MAEMO_CHANGES
  guint padding_top;
  guint padding_bottom;
  guint padding_left;
  guint padding_right;
#endif
  
  dialog = GTK_DIALOG (object);

#ifdef MAEMO_CHANGES
  gtk_dialog_get_padding (dialog, &padding_top, &padding_bottom, &padding_left, &padding_right);
#endif

  switch (prop_id)
    {
    case PROP_HAS_SEPARATOR:
      g_value_set_boolean (value, dialog->separator != NULL);
      break;

#ifdef MAEMO_CHANGES
    case PROP_TOP_PADDING:
      g_value_set_uint (value, padding_top);
      break;
    case PROP_BOTTOM_PADDING:
      g_value_set_uint (value, padding_bottom);
      break;
    case PROP_LEFT_PADDING:
      g_value_set_uint (value, padding_left);
      break;
    case PROP_RIGHT_PADDING:
      g_value_set_uint (value, padding_right);
      break;
    case PROP_INNER_SPACING:
      g_value_set_int (value, gtk_dialog_get_inner_spacing (dialog));
      break;
#endif /* MAEMO_CHANGES */

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static gboolean
gtk_dialog_delete_event_handler (GtkWidget   *widget,
                                 GdkEventAny *event,
                                 gpointer     user_data)
{
  /* emit response signal */
  gtk_dialog_response (GTK_DIALOG (widget), GTK_RESPONSE_DELETE_EVENT);

  /* Do the destroy by default */
  return FALSE;
}

/* A far too tricky heuristic for getting the right initial
 * focus widget if none was set. What we do is we focus the first
 * widget in the tab chain, but if this results in the focus
 * ending up on one of the response widgets _other_ than the
 * default response, we focus the default response instead.
 *
 * Additionally, skip selectable labels when looking for the
 * right initial focus widget.
 */
static void
gtk_dialog_map (GtkWidget *widget)
{
  GtkWindow *window = GTK_WINDOW (widget);
  GtkDialog *dialog = GTK_DIALOG (widget);
  
  GTK_WIDGET_CLASS (gtk_dialog_parent_class)->map (widget);

  if (!window->focus_widget)
    {
      GList *children, *tmp_list;
      GtkWidget *first_focus = NULL;
      
      do 
	{
	  g_signal_emit_by_name (window, "move_focus", GTK_DIR_TAB_FORWARD);

	  if (first_focus == NULL)
	    first_focus = window->focus_widget;
	  else if (first_focus == window->focus_widget)
            break;
	  if (!GTK_IS_LABEL (window->focus_widget))
	    break;
          if (!gtk_label_get_current_uri (GTK_LABEL (window->focus_widget)))
            gtk_label_select_region (GTK_LABEL (window->focus_widget), 0, 0);
	}
      while (TRUE);

      tmp_list = children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));
      
      while (tmp_list)
	{
	  GtkWidget *child = tmp_list->data;
	  
	  if ((window->focus_widget == NULL || 
	       child == window->focus_widget) && 
	      child != window->default_widget &&
	      window->default_widget)
	    {
	      gtk_widget_grab_focus (window->default_widget);
	      break;
	    }
	  
	  tmp_list = tmp_list->next;
	}
      
      g_list_free (children);
    }
}

static void
gtk_dialog_style_set (GtkWidget *widget,
                      GtkStyle  *prev_style)
{
  update_spacings (GTK_DIALOG (widget));
}

#ifdef MAEMO_CHANGES
static G_GNUC_CONST GQuark
auto_resize_quark (void)
{
  static GQuark quark = 0;

  if (G_UNLIKELY (quark == 0))
    quark = g_quark_from_static_string ("gtk-dialog-widget-auto-resize");

  return quark;
}

static void
gtk_dialog_resize_button (GtkWidget *button,
                          gpointer  *data)
{
  /* Only resize widgets added with gtk_dialog_add_button () */
  if (g_object_get_qdata (G_OBJECT (button), auto_resize_quark ()))
    {
      gint width = GPOINTER_TO_INT (data);
      g_object_set (button, "width-request", width, NULL);
    }
}

static void
gtk_dialog_screen_size_changed_cb (GdkScreen *screen,
                                   GtkDialog *dialog)
{
  GtkDialogPrivate *priv = GET_PRIVATE (dialog);
  GtkWidget *parent = gtk_widget_get_parent (dialog->action_area);
  gint width = gdk_screen_get_width (screen);
  gboolean portrait = width < gdk_screen_get_height (screen);
  gint button_width, padding;

  g_object_ref (dialog->action_area);
  gtk_container_remove (GTK_CONTAINER (parent), dialog->action_area);

  if (portrait)
    {
      parent = dialog->vbox;
      button_width = width - 16 /* HILDON_MARGIN_DOUBLE */ * 2;
      padding = 16 /* HILDON_MARGIN_DOUBLE */;
    }
  else
    {
      parent = gtk_widget_get_parent (dialog->vbox);
      button_width = 174 /* HILDON_DIALOG_BUTTON_WIDTH */;
      padding = 0;
    }

  gtk_box_pack_end (GTK_BOX (parent), dialog->action_area, FALSE, TRUE, padding);
  gtk_box_reorder_child (GTK_BOX (parent), dialog->action_area, 0);
  gtk_container_foreach (GTK_CONTAINER (dialog->action_area),
                         (GtkCallback) gtk_dialog_resize_button,
                         GINT_TO_POINTER (button_width));
  g_object_unref (dialog->action_area);

  if (portrait)
    gtk_box_set_spacing (GTK_BOX (priv->hbox), padding);
  else
    update_spacings (dialog);

  gtk_window_resize (GTK_WINDOW (dialog), 1, 1);
}

static void
gtk_dialog_realize (GtkWidget *widget)
{
  GdkScreen *screen = gtk_widget_get_screen (widget);

  GTK_WIDGET_CLASS (gtk_dialog_parent_class)->realize (widget);

  if (gdk_screen_get_width (screen) < gdk_screen_get_height (screen))
    gtk_dialog_screen_size_changed_cb (screen, GTK_DIALOG (widget));
  g_signal_connect (screen, "size-changed",
    G_CALLBACK (gtk_dialog_screen_size_changed_cb), widget);
}

static void
gtk_dialog_unrealize (GtkWidget *widget)
{
  GdkScreen *screen = gtk_widget_get_screen (widget);

  g_signal_handlers_disconnect_by_func (screen,
    gtk_dialog_screen_size_changed_cb, widget);

  GTK_WIDGET_CLASS (gtk_dialog_parent_class)->unrealize (widget);
}

static void
gtk_dialog_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
  GtkWindow *window;
  GdkWindowTypeHint type_hint;

  GTK_WIDGET_CLASS (gtk_dialog_parent_class)->size_request (widget, requisition);

  window = GTK_WINDOW (widget);
  type_hint = gtk_window_get_type_hint (window);

  if (window->type == GTK_WINDOW_TOPLEVEL &&
      (type_hint == GDK_WINDOW_TYPE_HINT_NORMAL ||
       type_hint == GDK_WINDOW_TYPE_HINT_DIALOG))
    {
      gint width;

      gtk_widget_get_size_request (widget, &width, NULL);

      if (width == -1)
        {
          GdkScreen *screen = gtk_widget_get_screen (widget);
          requisition->width = gdk_screen_get_width (screen);
        }
    }
}
#endif

static GtkWidget *
dialog_find_button (GtkDialog *dialog,
		    gint       response_id)
{
  GList *children, *tmp_list;
  GtkWidget *child = NULL;
      
  children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));

  for (tmp_list = children; tmp_list; tmp_list = tmp_list->next)
    {
      ResponseData *rd = get_response_data (tmp_list->data, FALSE);
      
      if (rd && rd->response_id == response_id)
	{
	  child = tmp_list->data;
	  break;
	}
    }

  g_list_free (children);

  return child;
}

static void
gtk_dialog_close (GtkDialog *dialog)
{
  /* Synthesize delete_event to close dialog. */
  
  GtkWidget *widget = GTK_WIDGET (dialog);
  GdkEvent *event;

  event = gdk_event_new (GDK_DELETE);
  
  event->any.window = g_object_ref (widget->window);
  event->any.send_event = TRUE;
  
  gtk_main_do_event (event);
  gdk_event_free (event);
}

GtkWidget*
gtk_dialog_new (void)
{
  return g_object_new (GTK_TYPE_DIALOG, NULL);
}

static GtkWidget*
gtk_dialog_new_empty (const gchar     *title,
                      GtkWindow       *parent,
                      GtkDialogFlags   flags)
{
  GtkDialog *dialog;

  dialog = g_object_new (GTK_TYPE_DIALOG, NULL);

  if (title)
    gtk_window_set_title (GTK_WINDOW (dialog), title);

  if (parent)
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

  if (flags & GTK_DIALOG_MODAL)
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
  
  if (flags & GTK_DIALOG_DESTROY_WITH_PARENT)
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

  if (flags & GTK_DIALOG_NO_SEPARATOR)
    gtk_dialog_set_has_separator (dialog, FALSE);
  
  return GTK_WIDGET (dialog);
}

/**
 * gtk_dialog_new_with_buttons:
 * @title: (allow-none): Title of the dialog, or %NULL
 * @parent: (allow-none): Transient parent of the dialog, or %NULL
 * @flags: from #GtkDialogFlags
 * @first_button_text: (allow-none): stock ID or text to go in first button, or %NULL
 * @Varargs: response ID for first button, then additional buttons, ending with %NULL
 *
 * Creates a new #GtkDialog with title @title (or %NULL for the default
 * title; see gtk_window_set_title()) and transient parent @parent (or
 * %NULL for none; see gtk_window_set_transient_for()). The @flags
 * argument can be used to make the dialog modal (#GTK_DIALOG_MODAL)
 * and/or to have it destroyed along with its transient parent
 * (#GTK_DIALOG_DESTROY_WITH_PARENT). After @flags, button
 * text/response ID pairs should be listed, with a %NULL pointer ending
 * the list. Button text can be either a stock ID such as
 * #GTK_STOCK_OK, or some arbitrary text. A response ID can be
 * any positive number, or one of the values in the #GtkResponseType
 * enumeration. If the user clicks one of these dialog buttons,
 * #GtkDialog will emit the #GtkDialog::response signal with the corresponding
 * response ID. If a #GtkDialog receives the #GtkWidget::delete-event signal, 
 * it will emit ::response with a response ID of #GTK_RESPONSE_DELETE_EVENT.
 * However, destroying a dialog does not emit the ::response signal;
 * so be careful relying on ::response when using the 
 * #GTK_DIALOG_DESTROY_WITH_PARENT flag. Buttons are from left to right,
 * so the first button in the list will be the leftmost button in the dialog.
 *
 * Here's a simple example:
 * |[
 *  GtkWidget *dialog = gtk_dialog_new_with_buttons ("My dialog",
 *                                                   main_app_window,
 *                                                   GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
 *                                                   GTK_STOCK_OK,
 *                                                   GTK_RESPONSE_ACCEPT,
 *                                                   GTK_STOCK_CANCEL,
 *                                                   GTK_RESPONSE_REJECT,
 *                                                   NULL);
 * ]|
 * 
 * Return value: a new #GtkDialog
 **/
GtkWidget*
gtk_dialog_new_with_buttons (const gchar    *title,
                             GtkWindow      *parent,
                             GtkDialogFlags  flags,
                             const gchar    *first_button_text,
                             ...)
{
  GtkDialog *dialog;
  va_list args;
  
  dialog = GTK_DIALOG (gtk_dialog_new_empty (title, parent, flags));

  va_start (args, first_button_text);

  gtk_dialog_add_buttons_valist (dialog,
                                 first_button_text,
                                 args);
  
  va_end (args);

  return GTK_WIDGET (dialog);
}

static void 
response_data_free (gpointer data)
{
  g_slice_free (ResponseData, data);
}

static ResponseData*
get_response_data (GtkWidget *widget,
		   gboolean   create)
{
  ResponseData *ad = g_object_get_data (G_OBJECT (widget),
                                        "gtk-dialog-response-data");

  if (ad == NULL && create)
    {
      ad = g_slice_new (ResponseData);
      
      g_object_set_data_full (G_OBJECT (widget),
                              I_("gtk-dialog-response-data"),
                              ad,
			      response_data_free);
    }

  return ad;
}

static void
action_widget_activated (GtkWidget *widget, GtkDialog *dialog)
{
  gint response_id;
  
  response_id = gtk_dialog_get_response_for_widget (dialog, widget);

  gtk_dialog_response (dialog, response_id);
}

/**
 * gtk_dialog_add_action_widget:
 * @dialog: a #GtkDialog
 * @child: an activatable widget
 * @response_id: response ID for @child
 * 
 * Adds an activatable widget to the action area of a #GtkDialog,
 * connecting a signal handler that will emit the #GtkDialog::response 
 * signal on the dialog when the widget is activated. The widget is 
 * appended to the end of the dialog's action area. If you want to add a
 * non-activatable widget, simply pack it into the @action_area field 
 * of the #GtkDialog struct.
 **/
void
gtk_dialog_add_action_widget (GtkDialog *dialog,
                              GtkWidget *child,
                              gint       response_id)
{
  ResponseData *ad;
  guint signal_id;
  
  g_return_if_fail (GTK_IS_DIALOG (dialog));
  g_return_if_fail (GTK_IS_WIDGET (child));

  ad = get_response_data (child, TRUE);

  ad->response_id = response_id;

  if (GTK_IS_BUTTON (child))
    signal_id = g_signal_lookup ("clicked", GTK_TYPE_BUTTON);
  else
    signal_id = GTK_WIDGET_GET_CLASS (child)->activate_signal;

  if (signal_id)
    {
      GClosure *closure;

      closure = g_cclosure_new_object (G_CALLBACK (action_widget_activated),
				       G_OBJECT (dialog));
      g_signal_connect_closure_by_id (child,
				      signal_id,
				      0,
				      closure,
				      FALSE);
    }
  else
    g_warning ("Only 'activatable' widgets can be packed into the action area of a GtkDialog");

  gtk_box_pack_end (GTK_BOX (dialog->action_area),
                    child,
                    FALSE, TRUE, 0);
  
  if (response_id == GTK_RESPONSE_HELP)
    gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (dialog->action_area), child, TRUE);
}
#ifdef MAEMO_CHANGES
  /* Buttons on a HildonDialog have fixed size */
  #define HILDON_DIALOG_BUTTON_WIDTH 174
#endif

/**
 * gtk_dialog_add_button:
 * @dialog: a #GtkDialog
 * @button_text: text of button, or stock ID
 * @response_id: response ID for the button
 * 
 * Adds a button with the given text (or a stock button, if @button_text is a
 * stock ID) and sets things up so that clicking the button will emit the
 * #GtkDialog::response signal with the given @response_id. The button is 
 * appended to the end of the dialog's action area. The button widget is 
 * returned, but usually you don't need it.
 *
 * Return value: (transfer none): the button widget that was added
 **/
GtkWidget*
gtk_dialog_add_button (GtkDialog   *dialog,
                       const gchar *button_text,
                       gint         response_id)
{
  GtkWidget *button;
  
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);
  g_return_val_if_fail (button_text != NULL, NULL);

  button = gtk_button_new_from_stock (button_text);

  gtk_widget_set_can_default (button, TRUE);
  
#ifdef MAEMO_CHANGES
  if (response_id != GTK_RESPONSE_CANCEL &&
      response_id != GTK_RESPONSE_REJECT &&
      response_id != GTK_RESPONSE_CLOSE)
    {
      gtk_widget_show (button);
    }
    else
    {
      gtk_widget_set_no_show_all (button, TRUE);
    }
    hildon_gtk_widget_set_theme_size (button, HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT);
    g_object_set (button, "width-request", HILDON_DIALOG_BUTTON_WIDTH, NULL);
    g_object_set_qdata (G_OBJECT (button), auto_resize_quark (), GINT_TO_POINTER (TRUE));
#else
  gtk_widget_show (button);
#endif /* MAEMO_CHANGES */
  
  gtk_dialog_add_action_widget (dialog,
                                button,
                                response_id);

  return button;
}

static void
gtk_dialog_add_buttons_valist (GtkDialog      *dialog,
                               const gchar    *first_button_text,
                               va_list         args)
{
  const gchar* text;
  gint response_id;

  g_return_if_fail (GTK_IS_DIALOG (dialog));
  
  if (first_button_text == NULL)
    return;
  
  text = first_button_text;
  response_id = va_arg (args, gint);

  while (text != NULL)
    {
      gtk_dialog_add_button (dialog, text, response_id);

      text = va_arg (args, gchar*);
      if (text == NULL)
        break;
      response_id = va_arg (args, int);
    }
}

/**
 * gtk_dialog_add_buttons:
 * @dialog: a #GtkDialog
 * @first_button_text: button text or stock ID
 * @Varargs: response ID for first button, then more text-response_id pairs
 * 
 * Adds more buttons, same as calling gtk_dialog_add_button()
 * repeatedly.  The variable argument list should be %NULL-terminated
 * as with gtk_dialog_new_with_buttons(). Each button must have both
 * text and response ID.
 **/
void
gtk_dialog_add_buttons (GtkDialog   *dialog,
                        const gchar *first_button_text,
                        ...)
{  
  va_list args;

  va_start (args, first_button_text);

  gtk_dialog_add_buttons_valist (dialog,
                                 first_button_text,
                                 args);
  
  va_end (args);
}

/**
 * gtk_dialog_set_response_sensitive:
 * @dialog: a #GtkDialog
 * @response_id: a response ID
 * @setting: %TRUE for sensitive
 *
 * Calls <literal>gtk_widget_set_sensitive (widget, @setting)</literal> 
 * for each widget in the dialog's action area with the given @response_id.
 * A convenient way to sensitize/desensitize dialog buttons.
 **/
void
gtk_dialog_set_response_sensitive (GtkDialog *dialog,
                                   gint       response_id,
                                   gboolean   setting)
{
  GList *children;
  GList *tmp_list;

  g_return_if_fail (GTK_IS_DIALOG (dialog));

  children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));

  tmp_list = children;
  while (tmp_list != NULL)
    {
      GtkWidget *widget = tmp_list->data;
      ResponseData *rd = get_response_data (widget, FALSE);

      if (rd && rd->response_id == response_id)
        gtk_widget_set_sensitive (widget, setting);

      tmp_list = g_list_next (tmp_list);
    }

  g_list_free (children);
}

/**
 * gtk_dialog_set_default_response:
 * @dialog: a #GtkDialog
 * @response_id: a response ID
 * 
 * Sets the last widget in the dialog's action area with the given @response_id
 * as the default widget for the dialog. Pressing "Enter" normally activates
 * the default widget.
 **/
void
gtk_dialog_set_default_response (GtkDialog *dialog,
                                 gint       response_id)
{
  GList *children;
  GList *tmp_list;

  g_return_if_fail (GTK_IS_DIALOG (dialog));

  children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));

  tmp_list = children;
  while (tmp_list != NULL)
    {
      GtkWidget *widget = tmp_list->data;
      ResponseData *rd = get_response_data (widget, FALSE);

      if (rd && rd->response_id == response_id)
	gtk_widget_grab_default (widget);
	    
      tmp_list = g_list_next (tmp_list);
    }

  g_list_free (children);
}

/**
 * gtk_dialog_set_has_separator:
 * @dialog: a #GtkDialog
 * @setting: %TRUE to have a separator
 *
 * Sets whether the dialog has a separator above the buttons.
 *
 * Deprecated: 2.22: This function will be removed in GTK+ 3
 **/
void
gtk_dialog_set_has_separator (GtkDialog *dialog,
                              gboolean   setting)
{
  GtkDialogPrivate *priv;

  g_return_if_fail (GTK_IS_DIALOG (dialog));

  priv = GET_PRIVATE (dialog);

  /* this might fail if we get called before _init() somehow */
  g_assert (dialog->vbox != NULL);

  if (priv->ignore_separator)
    {
      g_warning ("Ignoring the separator setting");
      return;
    }
  
  if (setting && dialog->separator == NULL)
    {
#ifdef MAEMO_CHANGES
      dialog->separator = gtk_vseparator_new ();
      gtk_box_pack_end (GTK_BOX (priv->hbox), dialog->separator, FALSE, TRUE, 0);

      /* The app programmer could screw this up, but, their own fault.
       * Moves the separator just above the action area.
       */
      gtk_box_reorder_child (GTK_BOX (priv->hbox), dialog->separator, 1);
#else
      dialog->separator = gtk_hseparator_new ();
      gtk_box_pack_end (GTK_BOX (dialog->vbox), dialog->separator, FALSE, TRUE, 0);

      /* The app programmer could screw this up, but, their own fault.
       * Moves the separator just above the action area.
       */
      gtk_box_reorder_child (GTK_BOX (dialog->vbox), dialog->separator, 1);
#endif /* MAEMO_CHANGES */
      gtk_widget_show (dialog->separator);
    }
  else if (!setting && dialog->separator != NULL)
    {
      gtk_widget_destroy (dialog->separator);
      dialog->separator = NULL;
    }

  g_object_notify (G_OBJECT (dialog), "has-separator");
}

/**
 * gtk_dialog_get_has_separator:
 * @dialog: a #GtkDialog
 * 
 * Accessor for whether the dialog has a separator.
 * 
 * Return value: %TRUE if the dialog has a separator
 *
 * Deprecated: 2.22: This function will be removed in GTK+ 3
 **/
gboolean
gtk_dialog_get_has_separator (GtkDialog *dialog)
{
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), FALSE);

  return dialog->separator != NULL;
}

#ifdef MAEMO_CHANGES
/**
 * gtk_dialog_set_padding:
 * @dialog: a #GtkDialog
 * @top_padding: Padding to add at the top of the dialog.
 * @bottom_padding: Padding to add at the bottom of the dialog.
 * @left_padding: Padding to add at the left of the dialog.
 * @right_padding: Padding to add at the right of the dialog.
 *
 * Sets additional padding around the dialog.
 *
 * Since: maemo 5.0
 **/
void
gtk_dialog_set_padding (GtkDialog *dialog,
                        guint      top_padding,
                        guint      bottom_padding,
                        guint      left_padding,
                        guint      right_padding)
{
  GtkDialogPrivate *priv;

  g_return_if_fail (GTK_IS_DIALOG (dialog));
  
  priv = GET_PRIVATE (dialog);
  
  gtk_alignment_set_padding (GTK_ALIGNMENT (priv->alignment), top_padding, bottom_padding, left_padding, right_padding);
}

/**
 * gtk_dialog_get_padding:
 * @dialog: a #GtkDialog
 * @padding_top: location to store the padding for the top of the dialog, or %NULL
 * @padding_bottom: location to store the padding for the bottom of the dialog, or %NULL
 * @padding_left: location to store the padding for the left of the dialog, or %NULL
 * @padding_right: location to store the padding for the right of the dialog, or %NULL
 *
 * Gets the padding on the different sides of the dialog.
 *
 * Since: maemo 5.0
 **/
void
gtk_dialog_get_padding (GtkDialog *dialog,
                        guint     *top_padding,
                        guint     *bottom_padding,
                        guint     *left_padding,
                        guint     *right_padding)
{
  GtkDialogPrivate *priv;

  g_return_if_fail (GTK_IS_DIALOG (dialog));

  priv = GET_PRIVATE (dialog);

  guint t, b, l, r;

  gtk_alignment_get_padding (GTK_ALIGNMENT (priv->alignment), &t, &b, &l, &r);

  if (top_padding)
    *top_padding = t;
  if (bottom_padding)
    *bottom_padding = b;
  if (left_padding)
    *left_padding = l;
  if (right_padding)
    *right_padding = r;
}

void
gtk_dialog_set_inner_spacing (GtkDialog *dialog,
                              guint      inner_spacing)
{
  GtkDialogPrivate *priv;
  g_return_if_fail (GTK_IS_DIALOG (dialog));

  priv = GET_PRIVATE (dialog);

  gtk_box_set_spacing (GTK_BOX (priv->hbox), inner_spacing);
}

guint
gtk_dialog_get_inner_spacing (GtkDialog *dialog)
{
  GtkDialogPrivate *priv;
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), 0);

  priv = GET_PRIVATE (dialog);

  return gtk_box_get_spacing (GTK_BOX (priv->hbox));
}

#endif /* MAEMO_CHANGES */

/**
 * gtk_dialog_response:
 * @dialog: a #GtkDialog
 * @response_id: response ID 
 * 
 * Emits the #GtkDialog::response signal with the given response ID. 
 * Used to indicate that the user has responded to the dialog in some way;
 * typically either you or gtk_dialog_run() will be monitoring the
 * ::response signal and take appropriate action.
 **/
void
gtk_dialog_response (GtkDialog *dialog,
                     gint       response_id)
{
  g_return_if_fail (GTK_IS_DIALOG (dialog));

  g_signal_emit (dialog,
		 dialog_signals[RESPONSE],
		 0,
		 response_id);
}

typedef struct
{
  GtkDialog *dialog;
  gint response_id;
  GMainLoop *loop;
  gboolean destroyed;
} RunInfo;

static void
shutdown_loop (RunInfo *ri)
{
  if (g_main_loop_is_running (ri->loop))
    g_main_loop_quit (ri->loop);
}

static void
run_unmap_handler (GtkDialog *dialog, gpointer data)
{
  RunInfo *ri = data;

  shutdown_loop (ri);
}

static void
run_response_handler (GtkDialog *dialog,
                      gint response_id,
                      gpointer data)
{
  RunInfo *ri;

  ri = data;

  ri->response_id = response_id;

  shutdown_loop (ri);
}

static gint
run_delete_handler (GtkDialog *dialog,
                    GdkEventAny *event,
                    gpointer data)
{
  RunInfo *ri = data;
    
  shutdown_loop (ri);
  
  return TRUE; /* Do not destroy */
}

static void
run_destroy_handler (GtkDialog *dialog, gpointer data)
{
  RunInfo *ri = data;

  /* shutdown_loop will be called by run_unmap_handler */
  
  ri->destroyed = TRUE;
}

/**
 * gtk_dialog_run:
 * @dialog: a #GtkDialog
 * 
 * Blocks in a recursive main loop until the @dialog either emits the
 * #GtkDialog::response signal, or is destroyed. If the dialog is 
 * destroyed during the call to gtk_dialog_run(), gtk_dialog_run() returns 
 * #GTK_RESPONSE_NONE. Otherwise, it returns the response ID from the 
 * ::response signal emission.
 *
 * Before entering the recursive main loop, gtk_dialog_run() calls
 * gtk_widget_show() on the dialog for you. Note that you still
 * need to show any children of the dialog yourself.
 *
 * During gtk_dialog_run(), the default behavior of #GtkWidget::delete-event 
 * is disabled; if the dialog receives ::delete_event, it will not be
 * destroyed as windows usually are, and gtk_dialog_run() will return
 * #GTK_RESPONSE_DELETE_EVENT. Also, during gtk_dialog_run() the dialog 
 * will be modal. You can force gtk_dialog_run() to return at any time by
 * calling gtk_dialog_response() to emit the ::response signal. Destroying 
 * the dialog during gtk_dialog_run() is a very bad idea, because your 
 * post-run code won't know whether the dialog was destroyed or not.
 *
 * After gtk_dialog_run() returns, you are responsible for hiding or
 * destroying the dialog if you wish to do so.
 *
 * Typical usage of this function might be:
 * |[
 *   gint result = gtk_dialog_run (GTK_DIALOG (dialog));
 *   switch (result)
 *     {
 *       case GTK_RESPONSE_ACCEPT:
 *          do_application_specific_something ();
 *          break;
 *       default:
 *          do_nothing_since_dialog_was_cancelled ();
 *          break;
 *     }
 *   gtk_widget_destroy (dialog);
 * ]|
 * 
 * Note that even though the recursive main loop gives the effect of a
 * modal dialog (it prevents the user from interacting with other 
 * windows in the same window group while the dialog is run), callbacks 
 * such as timeouts, IO channel watches, DND drops, etc, <emphasis>will</emphasis> 
 * be triggered during a gtk_dialog_run() call.
 * 
 * Return value: response ID
 **/
gint
gtk_dialog_run (GtkDialog *dialog)
{
  RunInfo ri = { NULL, GTK_RESPONSE_NONE, NULL, FALSE };
  gboolean was_modal;
  gulong response_handler;
  gulong unmap_handler;
  gulong destroy_handler;
  gulong delete_handler;
  
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), -1);

  g_object_ref (dialog);

  was_modal = GTK_WINDOW (dialog)->modal;
  if (!was_modal)
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

  if (!gtk_widget_get_visible (GTK_WIDGET (dialog)))
    gtk_widget_show (GTK_WIDGET (dialog));
  
  response_handler =
    g_signal_connect (dialog,
                      "response",
                      G_CALLBACK (run_response_handler),
                      &ri);
  
  unmap_handler =
    g_signal_connect (dialog,
                      "unmap",
                      G_CALLBACK (run_unmap_handler),
                      &ri);
  
  delete_handler =
    g_signal_connect (dialog,
                      "delete-event",
                      G_CALLBACK (run_delete_handler),
                      &ri);
  
  destroy_handler =
    g_signal_connect (dialog,
                      "destroy",
                      G_CALLBACK (run_destroy_handler),
                      &ri);
  
  ri.loop = g_main_loop_new (NULL, FALSE);

  GDK_THREADS_LEAVE ();  
  g_main_loop_run (ri.loop);
  GDK_THREADS_ENTER ();  

  g_main_loop_unref (ri.loop);

  ri.loop = NULL;
  
  if (!ri.destroyed)
    {
      if (!was_modal)
        gtk_window_set_modal (GTK_WINDOW(dialog), FALSE);
      
      g_signal_handler_disconnect (dialog, response_handler);
      g_signal_handler_disconnect (dialog, unmap_handler);
      g_signal_handler_disconnect (dialog, delete_handler);
      g_signal_handler_disconnect (dialog, destroy_handler);
    }

  g_object_unref (dialog);

  return ri.response_id;
}

void
_gtk_dialog_set_ignore_separator (GtkDialog *dialog,
				  gboolean   ignore_separator)
{
  GtkDialogPrivate *priv;

  priv = GET_PRIVATE (dialog);
  priv->ignore_separator = ignore_separator;
}

/**
 * gtk_dialog_get_widget_for_response:
 * @dialog: a #GtkDialog
 * @response_id: the response ID used by the @dialog widget
 *
 * Gets the widget button that uses the given response ID in the action area
 * of a dialog.
 *
 * Returns: (transfer none):the @widget button that uses the given @response_id, or %NULL.
 *
 * Since: 2.20
 */
GtkWidget*
gtk_dialog_get_widget_for_response (GtkDialog *dialog,
				    gint       response_id)
{
  GList *children;
  GList *tmp_list;

  g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);

  children = gtk_container_get_children (GTK_CONTAINER (dialog->action_area));

  tmp_list = children;
  while (tmp_list != NULL)
    {
      GtkWidget *widget = tmp_list->data;
      ResponseData *rd = get_response_data (widget, FALSE);

      if (rd && rd->response_id == response_id)
        {
          g_list_free (children);
          return widget;
        }

      tmp_list = g_list_next (tmp_list);
    }

  g_list_free (children);

  return NULL;
}

/**
 * gtk_dialog_get_response_for_widget:
 * @dialog: a #GtkDialog
 * @widget: a widget in the action area of @dialog
 *
 * Gets the response id of a widget in the action area
 * of a dialog.
 *
 * Returns: the response id of @widget, or %GTK_RESPONSE_NONE
 *  if @widget doesn't have a response id set.
 *
 * Since: 2.8
 */
gint
gtk_dialog_get_response_for_widget (GtkDialog *dialog,
				    GtkWidget *widget)
{
  ResponseData *rd;

  rd = get_response_data (widget, FALSE);
  if (!rd)
    return GTK_RESPONSE_NONE;
  else
    return rd->response_id;
}

/**
 * gtk_alternative_dialog_button_order:
 * @screen: (allow-none): a #GdkScreen, or %NULL to use the default screen
 *
 * Returns %TRUE if dialogs are expected to use an alternative
 * button order on the screen @screen. See
 * gtk_dialog_set_alternative_button_order() for more details
 * about alternative button order. 
 *
 * If you need to use this function, you should probably connect
 * to the ::notify:gtk-alternative-button-order signal on the
 * #GtkSettings object associated to @screen, in order to be 
 * notified if the button order setting changes.
 *
 * Returns: Whether the alternative button order should be used
 *
 * Since: 2.6
 */
gboolean 
gtk_alternative_dialog_button_order (GdkScreen *screen)
{
  GtkSettings *settings;
  gboolean result;

  if (screen)
    settings = gtk_settings_get_for_screen (screen);
  else
    settings = gtk_settings_get_default ();
  
  g_object_get (settings,
		"gtk-alternative-button-order", &result, NULL);

  return result;
}

static void
gtk_dialog_set_alternative_button_order_valist (GtkDialog *dialog,
						gint       first_response_id,
						va_list    args)
{
  GtkWidget *child;
  gint response_id;
  gint position;

  response_id = first_response_id;
  position = 0;
  while (response_id != -1)
    {
      /* reorder child with response_id to position */
      child = dialog_find_button (dialog, response_id);
      gtk_box_reorder_child (GTK_BOX (dialog->action_area), child, position);

      response_id = va_arg (args, gint);
      position++;
    }
}

/**
 * gtk_dialog_set_alternative_button_order:
 * @dialog: a #GtkDialog
 * @first_response_id: a response id used by one @dialog's buttons
 * @Varargs: a list of more response ids of @dialog's buttons, terminated by -1
 *
 * Sets an alternative button order. If the 
 * #GtkSettings:gtk-alternative-button-order setting is set to %TRUE, 
 * the dialog buttons are reordered according to the order of the 
 * response ids passed to this function.
 *
 * By default, GTK+ dialogs use the button order advocated by the Gnome 
 * <ulink url="http://developer.gnome.org/projects/gup/hig/2.0/">Human 
 * Interface Guidelines</ulink> with the affirmative button at the far 
 * right, and the cancel button left of it. But the builtin GTK+ dialogs
 * and #GtkMessageDialog<!-- -->s do provide an alternative button order,
 * which is more suitable on some platforms, e.g. Windows.
 *
 * Use this function after adding all the buttons to your dialog, as the 
 * following example shows:
 * |[
 * cancel_button = gtk_dialog_add_button (GTK_DIALOG (dialog),
 *                                        GTK_STOCK_CANCEL,
 *                                        GTK_RESPONSE_CANCEL);
 *  
 * ok_button = gtk_dialog_add_button (GTK_DIALOG (dialog),
 *                                    GTK_STOCK_OK,
 *                                    GTK_RESPONSE_OK);
 *   
 * gtk_widget_grab_default (ok_button);
 *   
 * help_button = gtk_dialog_add_button (GTK_DIALOG (dialog),
 *                                      GTK_STOCK_HELP,
 *                                      GTK_RESPONSE_HELP);
 *  
 * gtk_dialog_set_alternative_button_order (GTK_DIALOG (dialog),
 *                                          GTK_RESPONSE_OK,
 *                                          GTK_RESPONSE_CANCEL,
 *                                          GTK_RESPONSE_HELP,
 *                                          -1);
 * ]|
 * 
 * Since: 2.6
 */
void 
gtk_dialog_set_alternative_button_order (GtkDialog *dialog,
					 gint       first_response_id,
					 ...)
{
  GdkScreen *screen;
  va_list args;
  
  g_return_if_fail (GTK_IS_DIALOG (dialog));

  screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
  if (!gtk_alternative_dialog_button_order (screen))
      return;

  va_start (args, first_response_id);

  gtk_dialog_set_alternative_button_order_valist (dialog,
						  first_response_id,
						  args);
  va_end (args);
}
/**
 * gtk_dialog_set_alternative_button_order_from_array:
 * @dialog: a #GtkDialog
 * @n_params: the number of response ids in @new_order
 * @new_order: (array length=n_params): an array of response ids of
 *     @dialog's buttons
 *
 * Sets an alternative button order. If the 
 * #GtkSettings:gtk-alternative-button-order setting is set to %TRUE, 
 * the dialog buttons are reordered according to the order of the 
 * response ids in @new_order.
 *
 * See gtk_dialog_set_alternative_button_order() for more information.
 *
 * This function is for use by language bindings.
 * 
 * Since: 2.6
 */
void 
gtk_dialog_set_alternative_button_order_from_array (GtkDialog *dialog,
                                                    gint       n_params,
                                                    gint      *new_order)
{
  GdkScreen *screen;
  GtkWidget *child;
  gint position;

  g_return_if_fail (GTK_IS_DIALOG (dialog));
  g_return_if_fail (new_order != NULL);

  screen = gtk_widget_get_screen (GTK_WIDGET (dialog));
  if (!gtk_alternative_dialog_button_order (screen))
      return;

  for (position = 0; position < n_params; position++)
  {
      /* reorder child with response_id to position */
      child = dialog_find_button (dialog, new_order[position]);
      gtk_box_reorder_child (GTK_BOX (dialog->action_area), child, position);
    }
}

typedef struct {
  gchar *widget_name;
  gchar *response_id;
} ActionWidgetInfo;

typedef struct {
  GtkDialog *dialog;
  GtkBuilder *builder;
  GSList *items;
  gchar *response;
} ActionWidgetsSubParserData;

static void
attributes_start_element (GMarkupParseContext *context,
			  const gchar         *element_name,
			  const gchar        **names,
			  const gchar        **values,
			  gpointer             user_data,
			  GError             **error)
{
  ActionWidgetsSubParserData *parser_data = (ActionWidgetsSubParserData*)user_data;
  guint i;

  if (strcmp (element_name, "action-widget") == 0)
    {
      for (i = 0; names[i]; i++)
	if (strcmp (names[i], "response") == 0)
	  parser_data->response = g_strdup (values[i]);
    }
  else if (strcmp (element_name, "action-widgets") == 0)
    return;
  else
    g_warning ("Unsupported tag for GtkDialog: %s\n", element_name);
}

static void
attributes_text_element (GMarkupParseContext *context,
			 const gchar         *text,
			 gsize                text_len,
			 gpointer             user_data,
			 GError             **error)
{
  ActionWidgetsSubParserData *parser_data = (ActionWidgetsSubParserData*)user_data;
  ActionWidgetInfo *item;

  if (!parser_data->response)
    return;

  item = g_new (ActionWidgetInfo, 1);
  item->widget_name = g_strndup (text, text_len);
  item->response_id = parser_data->response;
  parser_data->items = g_slist_prepend (parser_data->items, item);
  parser_data->response = NULL;
}

static const GMarkupParser attributes_parser =
  {
    attributes_start_element,
    NULL,
    attributes_text_element,
  };

static gboolean
gtk_dialog_buildable_custom_tag_start (GtkBuildable  *buildable,
				       GtkBuilder    *builder,
				       GObject       *child,
				       const gchar   *tagname,
				       GMarkupParser *parser,
				       gpointer      *data)
{
  ActionWidgetsSubParserData *parser_data;

  if (child)
    return FALSE;

  if (strcmp (tagname, "action-widgets") == 0)
    {
      parser_data = g_slice_new0 (ActionWidgetsSubParserData);
      parser_data->dialog = GTK_DIALOG (buildable);
      parser_data->items = NULL;

      *parser = attributes_parser;
      *data = parser_data;
      return TRUE;
    }

  return parent_buildable_iface->custom_tag_start (buildable, builder, child,
						   tagname, parser, data);
}

static void
gtk_dialog_buildable_custom_finished (GtkBuildable *buildable,
				      GtkBuilder   *builder,
				      GObject      *child,
				      const gchar  *tagname,
				      gpointer      user_data)
{
  GSList *l;
  ActionWidgetsSubParserData *parser_data;
  GObject *object;
  GtkDialog *dialog;
  ResponseData *ad;
  guint signal_id;
  
  if (strcmp (tagname, "action-widgets"))
    {
    parent_buildable_iface->custom_finished (buildable, builder, child,
					     tagname, user_data);
    return;
    }

  dialog = GTK_DIALOG (buildable);
  parser_data = (ActionWidgetsSubParserData*)user_data;
  parser_data->items = g_slist_reverse (parser_data->items);

  for (l = parser_data->items; l; l = l->next)
    {
      ActionWidgetInfo *item = l->data;

      object = gtk_builder_get_object (builder, item->widget_name);
      if (!object)
	{
	  g_warning ("Unknown object %s specified in action-widgets of %s",
		     item->widget_name,
		     gtk_buildable_get_name (GTK_BUILDABLE (buildable)));
	  continue;
	}

      ad = get_response_data (GTK_WIDGET (object), TRUE);
      ad->response_id = atoi (item->response_id);

      if (GTK_IS_BUTTON (object))
	signal_id = g_signal_lookup ("clicked", GTK_TYPE_BUTTON);
      else
	signal_id = GTK_WIDGET_GET_CLASS (object)->activate_signal;
      
      if (signal_id)
	{
	  GClosure *closure;
	  
	  closure = g_cclosure_new_object (G_CALLBACK (action_widget_activated),
					   G_OBJECT (dialog));
	  g_signal_connect_closure_by_id (object,
					  signal_id,
					  0,
					  closure,
					  FALSE);
	}

      if (ad->response_id == GTK_RESPONSE_HELP)
	gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (dialog->action_area),
					    GTK_WIDGET (object), TRUE);

      g_free (item->widget_name);
      g_free (item->response_id);
      g_free (item);
    }
  g_slist_free (parser_data->items);
  g_slice_free (ActionWidgetsSubParserData, parser_data);
}

/**
 * gtk_dialog_get_action_area:
 * @dialog: a #GtkDialog
 *
 * Returns the action area of @dialog.
 *
 * Returns: (transfer none): the action area.
 *
 * Since: 2.14
 **/
GtkWidget *
gtk_dialog_get_action_area (GtkDialog *dialog)
{
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);

  return dialog->action_area;
}

/**
 * gtk_dialog_get_content_area:
 * @dialog: a #GtkDialog
 *
 * Returns the content area of @dialog.
 *
 * Returns: (transfer none): the content area #GtkVBox.
 *
 * Since: 2.14
 **/
GtkWidget *
gtk_dialog_get_content_area (GtkDialog *dialog)
{
  g_return_val_if_fail (GTK_IS_DIALOG (dialog), NULL);

  return dialog->vbox;
}

#define __GTK_DIALOG_C__
#include "gtkaliasdef.c"
