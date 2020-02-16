/****************************************************************************
**
**
** KRDB - puts current KDE color scheme into preprocessor statements
** cats specially written application default files and uses xrdb -merge to
** write to RESOURCE_MANAGER. Thus it gives a  simple way to make non-KDE
** applications fit in with the desktop
**
** Copyright (C) 1998 by Mark Donohoe
** Copyright (C) 1999 by Dirk A. Mueller (reworked for KDE 2.0)
** Copyright (C) 2001 by Matthias Ettrich (add support for GTK applications )
** Copyright (C) 2001 by Waldo Bastian <bastian@kde.org>
** Copyright (C) 2002 by Karol Szwed <gallium@kde.org>
** This application is freely distributable under the GNU Public License.
**
*****************************************************************************/

//#include <config-workspace.h>
//#include <config-X11.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef Unsorted
#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QFontDatabase>
#include <QSettings>
#include <QTextCodec>

#include <QPixmap>
#include <QByteArray>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QDBusConnection>
#include <QSaveFile>
#include <QTemporaryFile>

#include <KColorUtils>
#include <KColorScheme>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kprocess.h>
//#include <KLocalizedString>
//#include <kdelibs4migration.h>
//#include <KWindowSystem>

//#include <updatelaunchenvjob.h>

#include "krdb.h"
//#if HAVE_X11
//#include <X11/Xlib.h>
//#include <QX11Info>
// /#endif





// ---------------------------------------------------------------------

QString gtkColorsHelper(const QString &name, const QString &color)
{
    return QStringLiteral("@define-color %1 %2;\n").arg(name, color);
}
void checkGtkCss()
{
  QFile gtkCss(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/gtk-3.0/gtk.css");
  if (gtkCss.open(QIODevice::ReadWrite)) {
      QTextStream gtkStream(&gtkCss);
      bool hasImport = false;
      while (!gtkStream.atEnd()) {
          QString line = gtkStream.readLine();
          if (line.contains("@import 'colors.css';")) {
              hasImport = true;
          }
      }
      if (!hasImport) {
          gtkStream << "@import 'colors.css';";
      }
  }
}
// -----------------------------------------------------------------------------
void exportGtkColors(QList<KColorScheme> activeColorSchemes, QList<KColorScheme> inactiveColorSchemes, QList<KColorScheme> disabledColorSchemes, KConfigGroup groupWMTheme, QTextStream& colorsStream)
{
  /* 
      Normal (Non Backdrop, Non Insensitive) 
  */

  // General Colors

  colorsStream << gtkColorsHelper("theme_fg_color", activeColorSchemes[1].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_bg_color", activeColorSchemes[1].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_text_color", activeColorSchemes[0].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_base_color", activeColorSchemes[0].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_view_hover_decoration_color", activeColorSchemes[0].decoration(KColorScheme::HoverColor).color().name());
  colorsStream << gtkColorsHelper("theme_hovering_selected_bg_color", activeColorSchemes[3].decoration(KColorScheme::HoverColor).color().name());
  colorsStream << gtkColorsHelper("theme_selected_bg_color", activeColorSchemes[3].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_selected_fg_color", activeColorSchemes[3].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_view_active_decoration_color", activeColorSchemes[0].decoration(KColorScheme::HoverColor).color().name());

  // Button Colors
  colorsStream << gtkColorsHelper("theme_button_background_normal", activeColorSchemes[2].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_hover", activeColorSchemes[2].decoration(KColorScheme::HoverColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_focus", activeColorSchemes[2].decoration(KColorScheme::FocusColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_normal", activeColorSchemes[2].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_active", activeColorSchemes[3].foreground(KColorScheme::NormalText).color().name());
  
  // Misc Colors
  QColor windowForegroundColor = activeColorSchemes[1].foreground(KColorScheme::NormalText).color();
  QColor windowBackgroundColor = activeColorSchemes[1].background(KColorScheme::NormalBackground).color();
  QColor bordersColor = KColorUtils::mix(windowBackgroundColor,windowForegroundColor, 0.25);

  colorsStream << gtkColorsHelper("borders", bordersColor.name());
  colorsStream << gtkColorsHelper("warning_color", activeColorSchemes[0].foreground(KColorScheme::NeutralText).color().name());
  colorsStream << gtkColorsHelper("success_color", activeColorSchemes[0].foreground(KColorScheme::PositiveText).color().name());
  colorsStream << gtkColorsHelper("error_color", activeColorSchemes[0].foreground(KColorScheme::NegativeText).color().name());

  /* 
      Backdrop (Inactive) 
  */

  // General
  colorsStream << gtkColorsHelper("theme_unfocused_fg_color",inactiveColorSchemes[1].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_text_color", inactiveColorSchemes[0].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_bg_color", inactiveColorSchemes[1].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_base_color", inactiveColorSchemes[0].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_selected_bg_color_alt", inactiveColorSchemes[3].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_selected_bg_color", inactiveColorSchemes[3].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_selected_fg_color", inactiveColorSchemes[3].foreground(KColorScheme::NormalText).color().name());

  // Button
  colorsStream << gtkColorsHelper("theme_button_background_backdrop", inactiveColorSchemes[2].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_hover_backdrop", inactiveColorSchemes[2].decoration(KColorScheme::HoverColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_focus_backdrop", inactiveColorSchemes[2].decoration(KColorScheme::FocusColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_backdrop", inactiveColorSchemes[2].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_active_backdrop", inactiveColorSchemes[3].foreground(KColorScheme::NormalText).color().name());

  // Misc Colors
  QColor inactiveWindowForegroundColor = inactiveColorSchemes[1].foreground(KColorScheme::NormalText).color();
  QColor inactiveWindowBackgroundColor = inactiveColorSchemes[1].background(KColorScheme::NormalBackground).color();
  QColor inactiveBordersColor = KColorUtils::mix(inactiveWindowBackgroundColor,inactiveWindowForegroundColor, 0.25);

  colorsStream << gtkColorsHelper("unfocused_borders", inactiveBordersColor.name());
  colorsStream << gtkColorsHelper("warning_color_backdrop", inactiveColorSchemes[0].foreground(KColorScheme::NeutralText).color().name());
  colorsStream << gtkColorsHelper("success_color_backdrop", inactiveColorSchemes[0].foreground(KColorScheme::PositiveText).color().name());
  colorsStream << gtkColorsHelper("error_color_backdrop", inactiveColorSchemes[0].foreground(KColorScheme::NegativeText).color().name());

  /* 
      Insensitive (Disabled) 
  */

  // General
  colorsStream << gtkColorsHelper("insensitive_fg_color",disabledColorSchemes[1].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("insensitive_base_fg_color", disabledColorSchemes[0].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("insensitive_bg_color", disabledColorSchemes[1].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("insensitive_base_color", disabledColorSchemes[0].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("insensitive_selected_bg_color", disabledColorSchemes[3].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("insensitive_selected_fg_color", disabledColorSchemes[3].foreground(KColorScheme::NormalText).color().name());

  // Button
  colorsStream << gtkColorsHelper("theme_button_background_insensitive", disabledColorSchemes[2].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_hover_insensitive", disabledColorSchemes[2].decoration(KColorScheme::HoverColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_focus_insensitive", disabledColorSchemes[2].decoration(KColorScheme::FocusColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_insensitive", disabledColorSchemes[2].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_active_insensitive", disabledColorSchemes[3].foreground(KColorScheme::NormalText).color().name());

  // Misc Colors
  QColor disabledWindowForegroundColor = disabledColorSchemes[1].foreground(KColorScheme::NormalText).color();
  QColor disabledWindowBackgroundColor = disabledColorSchemes[1].background(KColorScheme::NormalBackground).color();
  QColor disabledBordersColor = KColorUtils::mix(disabledWindowBackgroundColor,disabledWindowForegroundColor, 0.25);

  colorsStream << gtkColorsHelper("insensitive_borders", disabledBordersColor.name());
  colorsStream << gtkColorsHelper("warning_color_insensitive", disabledColorSchemes[0].foreground(KColorScheme::NeutralText).color().name());
  colorsStream << gtkColorsHelper("success_color_insensitive", disabledColorSchemes[0].foreground(KColorScheme::PositiveText).color().name());
  colorsStream << gtkColorsHelper("error_color_insensitive", disabledColorSchemes[0].foreground(KColorScheme::NegativeText).color().name());

  /* 
      Insensitive Backdrop (Inactive Disabled) These pretty much have the same appearance as regular inactive colors, but they're separate in case we decide to make
      them different in the future.
  */

  // General
  colorsStream << gtkColorsHelper("insensitive_unfocused_fg_color",disabledColorSchemes[1].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_view_text_color", disabledColorSchemes[0].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("insensitive_unfocused_bg_color", disabledColorSchemes[1].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_unfocused_view_bg_color", disabledColorSchemes[0].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("insensitive_unfocused_selected_bg_color", disabledColorSchemes[3].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("insensitive_unfocused_selected_fg_color", disabledColorSchemes[3].foreground(KColorScheme::NormalText).color().name());

  // Button
  colorsStream << gtkColorsHelper("theme_button_background_backdrop_insensitive", disabledColorSchemes[2].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_hover_backdrop_insensitive", disabledColorSchemes[2].decoration(KColorScheme::HoverColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_decoration_focus_backdrop_insensitive", disabledColorSchemes[2].decoration(KColorScheme::FocusColor).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_backdrop_insensitive", disabledColorSchemes[2].foreground(KColorScheme::NormalText).color().name());
  colorsStream << gtkColorsHelper("theme_button_foreground_active_backdrop_insensitive", disabledColorSchemes[3].foreground(KColorScheme::NormalText).color().name());

  // Misc Colors
  QColor unfocusedDisabledWindowForegroundColor = disabledColorSchemes[1].foreground(KColorScheme::NormalText).color();
  QColor unfocusedDisabledWindowBackgroundColor = disabledColorSchemes[1].background(KColorScheme::NormalBackground).color();
  QColor unfocusedDisabledBordersColor = KColorUtils::mix(unfocusedDisabledWindowBackgroundColor,unfocusedDisabledWindowForegroundColor, 0.25);

  colorsStream << gtkColorsHelper("unfocused_insensitive_borders", unfocusedDisabledBordersColor.name());
  colorsStream << gtkColorsHelper("warning_color_insensitive_backdrop", disabledColorSchemes[0].foreground(KColorScheme::NeutralText).color().name());
  colorsStream << gtkColorsHelper("success_color_insensitive_backdrop", disabledColorSchemes[0].foreground(KColorScheme::PositiveText).color().name());
  colorsStream << gtkColorsHelper("error_color_insensitive_backdrop", disabledColorSchemes[0].foreground(KColorScheme::NegativeText).color().name());

  /*
      Ignorant Colors (These colors do not care about backdrop or insensitive states)
  */
  
  colorsStream << gtkColorsHelper("link_color", activeColorSchemes[0].foreground(KColorScheme::LinkText).color().name());
  colorsStream << gtkColorsHelper("link_visited_color", activeColorSchemes[0].foreground(KColorScheme::VisitedText).color().name());

  QColor tooltipForegroundColor = activeColorSchemes[4].foreground(KColorScheme::NormalText).color();
  QColor tooltipBackgroundColor = activeColorSchemes[4].background(KColorScheme::NormalBackground).color();
  QColor tooltipBorderColor = KColorUtils::mix(tooltipBackgroundColor, tooltipForegroundColor, 0.25);

  colorsStream << gtkColorsHelper("tooltip_text", tooltipForegroundColor.name());
  colorsStream << gtkColorsHelper("tooltip_background", tooltipBackgroundColor.name());
  colorsStream << gtkColorsHelper("tooltip_border", tooltipBorderColor.name());

  colorsStream << gtkColorsHelper("content_view_bg", activeColorSchemes[0].background(KColorScheme::NormalBackground).color().name());

  // Titlebar colors
  colorsStream << gtkColorsHelper("theme_titlebar_background", "rgb(" + groupWMTheme.readEntry("activeBackground", "") + QLatin1Char(')'));
  colorsStream << gtkColorsHelper("theme_titlebar_foreground", "rgb(" + groupWMTheme.readEntry("activeForeground", "") + QLatin1Char(')'));
  colorsStream << gtkColorsHelper("theme_titlebar_background_light", activeColorSchemes[1].background(KColorScheme::NormalBackground).color().name());
  colorsStream << gtkColorsHelper("theme_titlebar_foreground_backdrop", "rgb(" + groupWMTheme.readEntry("inactiveForeground", "") + QLatin1Char(')'));
  colorsStream << gtkColorsHelper("theme_titlebar_background_backdrop", "rgb(" + groupWMTheme.readEntry("inactiveBackground", "") + QLatin1Char(')'));
  colorsStream << gtkColorsHelper("theme_titlebar_foreground_insensitive", "rgb(" + groupWMTheme.readEntry("inactiveForeground", "") + QLatin1Char(')'));
  colorsStream << gtkColorsHelper("theme_titlebar_foreground_insensitive_backdrop", "rgb(" + groupWMTheme.readEntry("inactiveForeground", "") + QLatin1Char(')'));
}
// -----------------------------------------------------------------------------
void saveGtkColors()
{
    checkGtkCss();
    QFile colorsCss(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/gtk-3.0/colors.css");
    KConfigGroup groupWMTheme(KSharedConfig::openConfig(), "WM");

    if (colorsCss.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream colorsStream(&colorsCss);
        /*
        0 Active View
        1 Active Window
        2 Active Button
        3 Active Selection
        4 Active Tooltip
        5 Active Complimentary
        */
        
        KSharedConfigPtr config = KSharedConfig::openConfig();
        //KSharedConfig::openConfig("~/.local/share/konsole/Solarized.colorscheme", KConfig::SimpleConfig);
        QList<KColorScheme> activeColorSchemes{
            KColorScheme(QPalette::Active, KColorScheme::View,config),
            KColorScheme(QPalette::Active, KColorScheme::Window,config),
            KColorScheme(QPalette::Active, KColorScheme::Button,config),
            KColorScheme(QPalette::Active, KColorScheme::Selection,config),
            KColorScheme(QPalette::Active, KColorScheme::Tooltip,config),
            KColorScheme(QPalette::Active, KColorScheme::Complementary,config)
        };
        QList<KColorScheme> inactiveColorSchemes{
            KColorScheme(QPalette::Inactive, KColorScheme::View,config),
            KColorScheme(QPalette::Inactive, KColorScheme::Window,config),
            KColorScheme(QPalette::Inactive, KColorScheme::Button,config),
            KColorScheme(QPalette::Inactive, KColorScheme::Selection,config),
            KColorScheme(QPalette::Inactive, KColorScheme::Tooltip,config),
            KColorScheme(QPalette::Inactive, KColorScheme::Complementary,config)
        };
        QList<KColorScheme> disabledColorSchemes{
            KColorScheme(QPalette::Disabled, KColorScheme::View,config),
            KColorScheme(QPalette::Disabled, KColorScheme::Window,config),
            KColorScheme(QPalette::Disabled, KColorScheme::Button,config),
            KColorScheme(QPalette::Disabled, KColorScheme::Selection,config),
            KColorScheme(QPalette::Disabled, KColorScheme::Tooltip,config),
            KColorScheme(QPalette::Disabled, KColorScheme::Complementary,config)
        };

        exportGtkColors(activeColorSchemes, inactiveColorSchemes, disabledColorSchemes, groupWMTheme, colorsStream);
    }
}

// -----------------------------------------------------------------------------

static void applyQtColors( KSharedConfigPtr kglobalcfg, QSettings& settings, QPalette& newPal )
{
  QStringList actcg, inactcg, discg;
  /* export kde color settings */
  int i;
  for (i = 0; i < QPalette::NColorRoles; i++)
     actcg   << newPal.color(QPalette::Active,
                (QPalette::ColorRole) i).name();
  for (i = 0; i < QPalette::NColorRoles; i++)
     inactcg << newPal.color(QPalette::Inactive,
                (QPalette::ColorRole) i).name();
  for (i = 0; i < QPalette::NColorRoles; i++)
     discg   << newPal.color(QPalette::Disabled,
                (QPalette::ColorRole) i).name();

  settings.setValue(QStringLiteral("/qt/Palette/active"), actcg);
  settings.setValue(QStringLiteral("/qt/Palette/inactive"), inactcg);
  settings.setValue(QStringLiteral("/qt/Palette/disabled"), discg);

  // export kwin's colors to qtrc for kstyle to use
  KConfigGroup wmCfgGroup(kglobalcfg, "WM");

  // active colors
  QColor clr = newPal.color( QPalette::Active, QPalette::Window );
  clr = wmCfgGroup.readEntry("activeBackground", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/activeBackground"), clr.name());
  if (QPixmap::defaultDepth() > 8)
    clr = clr.darker(110);
  clr = wmCfgGroup.readEntry("activeBlend", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/activeBlend"), clr.name());
  clr = newPal.color( QPalette::Active, QPalette::HighlightedText );
  clr = wmCfgGroup.readEntry("activeForeground", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/activeForeground"), clr.name());
  clr = newPal.color( QPalette::Active,QPalette::Window );
  clr = wmCfgGroup.readEntry("frame", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/frame"), clr.name());
  clr = wmCfgGroup.readEntry("activeTitleBtnBg", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/activeTitleBtnBg"), clr.name());

  // inactive colors
  clr = newPal.color(QPalette::Inactive, QPalette::Window);
  clr = wmCfgGroup.readEntry("inactiveBackground", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/inactiveBackground"), clr.name());
  if (QPixmap::defaultDepth() > 8)
    clr = clr.darker(110);
  clr = wmCfgGroup.readEntry("inactiveBlend", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/inactiveBlend"), clr.name());
  clr = newPal.color(QPalette::Inactive, QPalette::Window).darker();
  clr = wmCfgGroup.readEntry("inactiveForeground", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/inactiveForeground"), clr.name());
  clr = newPal.color(QPalette::Inactive, QPalette::Window);
  clr = wmCfgGroup.readEntry("inactiveFrame", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/inactiveFrame"), clr.name());
  clr = wmCfgGroup.readEntry("inactiveTitleBtnBg", clr);
  settings.setValue(QStringLiteral("/qt/KWinPalette/inactiveTitleBtnBg"), clr.name());

  KConfigGroup kdeCfgGroup(kglobalcfg, "KDE");
  settings.setValue(QStringLiteral("/qt/KDE/contrast"), kdeCfgGroup.readEntry("contrast", 7));
}

// -----------------------------------------------------------------------------

static void applyQtSettings( KSharedConfigPtr kglobalcfg, QSettings& settings )
{
    /* export font settings */

    // NOTE keep this in sync with kfontsettingsdata in plasma-integration (cf. also Bug 378262)
    QFont defaultFont(QStringLiteral("Noto Sans"), 10, -1);
    defaultFont.setStyleHint(QFont::SansSerif);

    const KConfigGroup configGroup(KSharedConfig::openConfig(), QStringLiteral("General"));
    const QString fontInfo = configGroup.readEntry(QStringLiteral("font"), QString());
    if (!fontInfo.isEmpty()) {
        defaultFont.fromString(fontInfo);
    }

    settings.setValue(QStringLiteral("/qt/font"), defaultFont.toString());

  /* export effects settings */
  KConfigGroup kdeCfgGroup(kglobalcfg, "General");
  bool effectsEnabled = kdeCfgGroup.readEntry("EffectsEnabled", false);
  bool fadeMenus = kdeCfgGroup.readEntry("EffectFadeMenu", false);
  bool fadeTooltips = kdeCfgGroup.readEntry("EffectFadeTooltip", false);
  bool animateCombobox = kdeCfgGroup.readEntry("EffectAnimateCombo", false);

  QStringList guieffects;
  if (effectsEnabled) {
    guieffects << QStringLiteral("general");
    if (fadeMenus)
      guieffects << QStringLiteral("fademenu");
    if (animateCombobox)
      guieffects << QStringLiteral("animatecombo");
    if (fadeTooltips)
      guieffects << QStringLiteral("fadetooltip");
  }
  else
    guieffects << QStringLiteral("none");

  settings.setValue(QStringLiteral("/qt/GUIEffects"), guieffects);
}

// -----------------------------------------------------------------------------

static void addColorDef(QString& s, const char* n, const QColor& col)
{
  QString tmp;

  tmp.sprintf("#define %s #%02x%02x%02x\n",
              n, col.red(), col.green(), col.blue());

  s += tmp;
}


// -----------------------------------------------------------------------------

static void copyFile(QFile& tmp, QString const& filename, bool )
{
  QFile f( filename );
  if ( f.open(QIODevice::ReadOnly) ) {
      QByteArray buf( 8192, ' ' );
      while ( !f.atEnd() ) {
          int read = f.read( buf.data(), buf.size() );
          if ( read > 0 )
              tmp.write( buf.data(), read );
      }
  }
}


// -----------------------------------------------------------------------------

//static QString item( int i ) {
//    return QString::number( i / 255.0, 'f', 3 );
//}

//static QString color( const QColor& col )
//{
    //return QStringLiteral( "{ %1, %2, %3 }" ).arg( item( col.red() ) ).arg( item( col.green() ) ).arg( item( col.blue() ) );
//}


// -----------------------------------------------------------------------------

void runRdb( uint flags )
{
  // Obtain the application palette that is about to be set.
  bool exportColors      = flags & KRdbExportColors;
  bool exportQtColors    = flags & KRdbExportQtColors;
  bool exportQtSettings  = flags & KRdbExportQtSettings;
  bool exportXftSettings = flags & KRdbExportXftSettings;
  bool exportGtkTheme    = flags & KRdbExportGtkTheme;
  bool exportGtkColors   = flags & KRdbExportGtkColors;

  KSharedConfigPtr kglobalcfg = KSharedConfig::openConfig( QStringLiteral("kdeglobals") );
  KConfigGroup kglobals(kglobalcfg, "KDE");
  QPalette newPal = KColorScheme::createApplicationPalette(kglobalcfg);

  QTemporaryFile tmpFile;
  if (!tmpFile.open())
  {
    qDebug() << "Couldn't open temp file";
    exit(0);
  }


  KConfigGroup generalCfgGroup(kglobalcfg, "General");

  QString gtkTheme;
  if (kglobals.hasKey("widgetStyle"))
    gtkTheme = kglobals.readEntry("widgetStyle");
  else
    gtkTheme = QStringLiteral("oxygen");



  // Export colors to non-(KDE/Qt) apps (e.g. Motif, GTK+ apps)
  if (exportColors)
  {
    KConfigGroup g(KSharedConfig::openConfig(), "WM");
    QString preproc;
    QColor backCol = newPal.color( QPalette::Active, QPalette::Window );
    addColorDef(preproc, "FOREGROUND"         , newPal.color( QPalette::Active, QPalette::WindowText ) );
    addColorDef(preproc, "BACKGROUND"         , backCol);
    addColorDef(preproc, "HIGHLIGHT"          , backCol.lighter(100+(2*KColorScheme::contrast()+4)*16/1));
    addColorDef(preproc, "LOWLIGHT"           , backCol.darker(100+(2*KColorScheme::contrast()+4)*10));
    addColorDef(preproc, "SELECT_BACKGROUND"  , newPal.color( QPalette::Active, QPalette::Highlight));
    addColorDef(preproc, "SELECT_FOREGROUND"  , newPal.color( QPalette::Active, QPalette::HighlightedText));
    addColorDef(preproc, "WINDOW_BACKGROUND"  , newPal.color( QPalette::Active, QPalette::Base ) );
    addColorDef(preproc, "WINDOW_FOREGROUND"  , newPal.color( QPalette::Active, QPalette::Text ) );
    addColorDef(preproc, "INACTIVE_BACKGROUND", g.readEntry("inactiveBackground", QColor(224, 223, 222)));
    addColorDef(preproc, "INACTIVE_FOREGROUND", g.readEntry("inactiveBackground", QColor(224, 223, 222)));
    addColorDef(preproc, "ACTIVE_BACKGROUND"  , g.readEntry("activeBackground", QColor(48, 174, 232)));
    addColorDef(preproc, "ACTIVE_FOREGROUND"  , g.readEntry("activeBackground", QColor(48, 174, 232)));
    //---------------------------------------------------------------

    tmpFile.write( preproc.toLatin1(), preproc.length() );

    QStringList list;

    const QStringList adPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
        QStringLiteral("kdisplay/app-defaults/"), QStandardPaths::LocateDirectory);
    for (QStringList::ConstIterator it = adPaths.constBegin(); it != adPaths.constEnd(); ++it) {
      QDir dSys( *it );

      if ( dSys.exists() ) {
        dSys.setFilter( QDir::Files );
        dSys.setSorting( QDir::Name );
        dSys.setNameFilters(QStringList(QStringLiteral("*.ad")));
        list += dSys.entryList();
      }
    }

    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it)
      copyFile(tmpFile, QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kdisplay/app-defaults/"+(*it)), true);

  }

  // Merge ~/.Xresources or fallback to ~/.Xdefaults
  QString homeDir = QDir::homePath();
  QString xResources = homeDir + "/.Xresources";

  // very primitive support for ~/.Xresources by appending it
  if ( QFile::exists( xResources ) )
    copyFile(tmpFile, xResources, true);
  else
    copyFile(tmpFile, homeDir + "/.Xdefaults", true);

  // Export the Xcursor theme & size settings
  KConfigGroup mousecfg(KSharedConfig::openConfig( QStringLiteral("kcminputrc") ), "Mouse" );
  QString theme = mousecfg.readEntry("cursorTheme", QString());
  QString size  = mousecfg.readEntry("cursorSize", QString());
  QString contents;

  if (!theme.isNull())
    contents = "Xcursor.theme: " + theme + '\n';

  if (!size.isNull())
    contents += "Xcursor.size: " + size + '\n';

  if (exportXftSettings)
  {
    contents += QLatin1String("Xft.antialias: ");
    if(generalCfgGroup.readEntry("XftAntialias", true))
      contents += QLatin1String("1\n");
    else
      contents += QLatin1String("0\n");

    QString hintStyle = generalCfgGroup.readEntry("XftHintStyle", "hintslight");
    contents += QLatin1String("Xft.hinting: ");
    if(hintStyle.isEmpty())
      contents += QLatin1String("-1\n");
    else
    {
      if(hintStyle!=QLatin1String("hintnone"))
        contents += QLatin1String("1\n");
      else
        contents += QLatin1String("0\n");
      contents += "Xft.hintstyle: " + hintStyle + '\n';
    }

    QString subPixel = generalCfgGroup.readEntry("XftSubPixel", "rgb");
    if(!subPixel.isEmpty())
      contents += "Xft.rgba: " + subPixel + '\n';

    KConfig _cfgfonts( QStringLiteral("kcmfonts") );
    KConfigGroup cfgfonts(&_cfgfonts, "General");

  }

  if (contents.length() > 0)
    tmpFile.write( contents.toLatin1(), contents.length() );

  tmpFile.flush();
  

  /* Qt exports */
  if ( exportQtColors || exportQtSettings )
  {
    QSettings* settings = new QSettings(QStringLiteral("Trolltech"));

    if ( exportQtColors )
      applyQtColors( kglobalcfg, *settings, newPal );    // For kcmcolors

    if ( exportQtSettings )
      applyQtSettings( kglobalcfg, *settings );          // For kcmstyle

    delete settings;
    QCoreApplication::processEvents();
#if HAVE_X11
    if (qApp->platformName() == QLatin1String("xcb")) {
        // We let KIPC take care of ourselves, as we are in a KDE app with
        // QApp::setDesktopSettingsAware(false);
        // Instead of calling QApp::x11_apply_settings() directly, we instead
        // modify the timestamp which propagates the settings changes onto
        // Qt-only apps without adversely affecting ourselves.

        // Cheat and use the current timestamp, since we just saved to qtrc.
        QDateTime settingsstamp = QDateTime::currentDateTime();

        static Atom qt_settings_timestamp = 0;
        if (!qt_settings_timestamp) {
            QString atomname(QStringLiteral("_QT_SETTINGS_TIMESTAMP_"));
            atomname += XDisplayName( nullptr ); // Use the $DISPLAY envvar.
            qt_settings_timestamp = XInternAtom( QX11Info::display(), atomname.toLatin1(), False);
        }

        QBuffer stamp;
        QDataStream s(&stamp.buffer(), QIODevice::WriteOnly);
        s << settingsstamp;
        XChangeProperty( QX11Info::display(), QX11Info::appRootWindow(), qt_settings_timestamp,
                        qt_settings_timestamp, 8, PropModeReplace,
                        (unsigned char*) stamp.buffer().data(),
                        stamp.buffer().size() );
        QApplication::flush();
    }
#endif
  }

   saveGtkColors();

}
