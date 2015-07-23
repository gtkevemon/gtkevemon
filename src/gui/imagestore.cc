// This file is part of GtkEveMon.
//
// GtkEveMon is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// You should have received a copy of the GNU General Public License
// along with GtkEveMon. If not, see <http://www.gnu.org/licenses/>.

#include <cmath>

#include "images/skill.h"
#include "images/certificate.h"
#include "images/skillstatus.h"
#include "images/skillicons.h"
#include "images/skillprogress.h"
#include "images/skilldeps.h"
#include "images/skillplan.h"
#include "images/skillqueue.h"
#include "images/certgrades.h"
#include "images/certstatus.h"
#include "images/eveportrait.h"
#include "images/applogo.h"
#include "images/aboutlogo.h"
#include "images/menuicons.h"
#include "images/guiimages.h"

#include "util/exception.h"

#include "imagestore.h"

#ifdef WIN32
# define round(x) ceil((x) - 0.5)
#endif

Glib::RefPtr<Gdk::Pixbuf> ImageStore::skill;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::skillqueue;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::certificate;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::certificate_small;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::applogo;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::aboutlogo;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::eveportrait;
Glib::RefPtr<Gdk::Pixbuf> ImageStore::columnconf[2];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::skillicons[6];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::skillstatus[8];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::skilldeps[3];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::skillplan[5];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::certgrades[4];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::certstatus[4];
Glib::RefPtr<Gdk::Pixbuf> ImageStore::menuicons[3];

/* ---------------------------------------------------------------- */

void
ImageStore::init (void)
{
  ImageStore::skill = ImageStore::create_from_inline(img_skill);
  ImageStore::skillqueue = ImageStore::create_from_inline
      (img_skillqueue);
  ImageStore::certificate = ImageStore::create_from_inline
      (img_certificate)->scale_simple(54, 54, Gdk::INTERP_BILINEAR);
  ImageStore::certificate_small = ImageStore::create_from_inline
      (img_certificate)->scale_simple(20, 20, Gdk::INTERP_BILINEAR);

  ImageStore::applogo = Gdk::Pixbuf::create_from_xpm_data
      (img_applogo_xpm);
  ImageStore::aboutlogo = Gdk::Pixbuf::create_from_xpm_data
      (img_aboutlogo_xpm);
  ImageStore::eveportrait = Gdk::Pixbuf::create_from_xpm_data
      (img_eveportrait_xpm);

  ImageStore::columnconf[0] = ImageStore::create_from_inline
      (img_columnconf);
  ImageStore::columnconf[1] = ImageStore::create_from_inline
      (img_columnconf_faded);

  ImageStore::skillicons[0] = ImageStore::create_from_inline
      (img_skillicons_group);
  ImageStore::skillicons[1] = ImageStore::create_from_inline
      (img_skillicons_skill);
  ImageStore::skillicons[2] = ImageStore::create_from_inline
      (img_skillicons_train);
  ImageStore::skillicons[3] = ImageStore::create_from_inline
      (img_skillicons_done);
  ImageStore::skillicons[4] = ImageStore::create_from_inline
      (img_skillicons_part);
  ImageStore::skillicons[5] = ImageStore::create_from_inline
      (img_skillicons_faded);

  ImageStore::skillstatus[0] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_nopre_xpm);
  ImageStore::skillstatus[1] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_havepre_xpm);
  ImageStore::skillstatus[2] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_at0_xpm);
  ImageStore::skillstatus[3] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_at1_xpm);
  ImageStore::skillstatus[4] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_at2_xpm);
  ImageStore::skillstatus[5] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_at3_xpm);
  ImageStore::skillstatus[6] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_at4_xpm);
  ImageStore::skillstatus[7] = Gdk::Pixbuf::create_from_xpm_data
      (img_skillstatus_at5_xpm);

  ImageStore::skilldeps[0] = ImageStore::create_from_inline
      (img_skilldeps_none);
  ImageStore::skilldeps[1] = ImageStore::create_from_inline
      (img_skilldeps_partial);
  ImageStore::skilldeps[2] = ImageStore::create_from_inline
      (img_skilldeps_have);

  ImageStore::skillplan[0] = ImageStore::create_from_inline
      (img_skillplan_trained);
  ImageStore::skillplan[1] = ImageStore::create_from_inline
      (img_skillplan_training);
  ImageStore::skillplan[2] = ImageStore::create_from_inline
      (img_skillplan_trainable);
  ImageStore::skillplan[3] = ImageStore::create_from_inline
      (img_skillplan_untrainable);
  ImageStore::skillplan[4] = ImageStore::create_from_inline
      (img_skillplan_deperror);

  ImageStore::certgrades[0] = Gdk::Pixbuf::create_from_xpm_data
      (img_certgrades_1_xpm);
  ImageStore::certgrades[1] = Gdk::Pixbuf::create_from_xpm_data
      (img_certgrades_2_xpm);
  ImageStore::certgrades[2] = Gdk::Pixbuf::create_from_xpm_data
      (img_certgrades_3_xpm);
  ImageStore::certgrades[3] = Gdk::Pixbuf::create_from_xpm_data
      (img_certgrades_4_xpm);

  ImageStore::certstatus[0] = ImageStore::create_from_inline
      (img_certstatus_claimed);
  ImageStore::certstatus[1] = ImageStore::create_from_inline
      (img_certstatus_claimable);
  ImageStore::certstatus[2] = ImageStore::create_from_inline
      (img_certstatus_partial);
  ImageStore::certstatus[3] = ImageStore::create_from_inline
      (img_certstatus_unavail);

  ImageStore::menuicons[0] = Gdk::Pixbuf::create_from_xpm_data
      (img_menu_evemon_xpm);
  ImageStore::menuicons[1] = Gdk::Pixbuf::create_from_xpm_data
      (img_menu_char_xpm);
  ImageStore::menuicons[2] = Gdk::Pixbuf::create_from_xpm_data
      (img_menu_help_xpm);
}

/* ---------------------------------------------------------------- */

Glib::RefPtr<Gdk::Pixbuf>
ImageStore::skill_progress (unsigned int level, double completed)
{
  Glib::RefPtr<Gdk::Pixbuf> ret(Gdk::Pixbuf::create_from_xpm_data
      (img_skillprogress_xpm));

  /* Some safety checks. */
  if (level > 5) level = 5;
  if (completed < 0.0) completed = 0.0;
  if (completed > 1.0) completed = 1.0;

  /* draw level */
  for (unsigned int l = 0; l < level; ++l)
    for (unsigned int x = 0; x < 6; ++x)
      ret->copy_area(0, 0, 1, 5, ret, (l * 7 + x + 2), 2);

  /* draw percent */
  for (unsigned int p = 0; p < (unsigned int)::round(34.0 * completed); ++p)
    ret->copy_area(0, 0, 1, 2, ret, (p + 2), 13);

  return ret;
}

/* ---------------------------------------------------------------- */

void
ImageStore::unload (void)
{
}

/* ---------------------------------------------------------------- */

Glib::RefPtr<Gdk::Pixbuf>
ImageStore::create_from_inline (guint8 const* data)
{
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  return Gdk::Pixbuf::create_from_inline(-1, data, false);
#else
  std::auto_ptr<Glib::Error> error;
  Glib::RefPtr<Gdk::Pixbuf> ret = Gdk::Pixbuf::create_from_inline
      (-1, data, false, error);
  if (error.get())
    throw error;
  return ret;
#endif
}
