#include <QString>
#include <QDebug>

#include <QStandardPaths>
#include <KSharedConfig>
#include <KColorScheme>
#include <KConfigGroup>

#include <QDBusConnection>
#include <QDBusMessage>

#include "krdb.h"

//this is a partial copy of kcm colors

int main() {
    //m_config->markAsClean();
    
    KSharedConfigPtr m_config = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));


    KConfigGroup group(m_config, "General");
    const QString selectedColorScheme = group.readEntry("ColorScheme");
    
    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,QStringLiteral("color-schemes/%1.colors").arg(selectedColorScheme));
    KSharedConfigPtr config = KSharedConfig::openConfig(path);
    
    
    //m_config->reparseConfiguration();
    
    const QStringList colorSetGroupList{
        QStringLiteral("Colors:View"),
        QStringLiteral("Colors:Window"),
        QStringLiteral("Colors:Button"),
        QStringLiteral("Colors:Selection"),
        QStringLiteral("Colors:Tooltip"),
        QStringLiteral("Colors:Complementary")
    };

    const QList<KColorScheme> colorSchemes{
        KColorScheme(QPalette::Active, KColorScheme::View, config),
        KColorScheme(QPalette::Active, KColorScheme::Window, config),
        KColorScheme(QPalette::Active, KColorScheme::Button, config),
        KColorScheme(QPalette::Active, KColorScheme::Selection, config),
        KColorScheme(QPalette::Active, KColorScheme::Tooltip, config),
        KColorScheme(QPalette::Active, KColorScheme::Complementary, config)
    };
    
    
    for (int i = 0; i < colorSchemes.length(); ++i) {
        KConfigGroup group(m_config, colorSetGroupList.value(i));
        group.writeEntry("BackgroundNormal", colorSchemes[i].background(KColorScheme::NormalBackground).color());
        group.writeEntry("BackgroundAlternate", colorSchemes[i].background(KColorScheme::AlternateBackground).color());
        group.writeEntry("ForegroundNormal", colorSchemes[i].foreground(KColorScheme::NormalText).color());
        group.writeEntry("ForegroundInactive", colorSchemes[i].foreground(KColorScheme::InactiveText).color());
        group.writeEntry("ForegroundActive", colorSchemes[i].foreground(KColorScheme::ActiveText).color());
        group.writeEntry("ForegroundLink", colorSchemes[i].foreground(KColorScheme::LinkText).color());
        group.writeEntry("ForegroundVisited", colorSchemes[i].foreground(KColorScheme::VisitedText).color());
        group.writeEntry("ForegroundNegative", colorSchemes[i].foreground(KColorScheme::NegativeText).color());
        group.writeEntry("ForegroundNeutral", colorSchemes[i].foreground(KColorScheme::NeutralText).color());
        group.writeEntry("ForegroundPositive", colorSchemes[i].foreground(KColorScheme::PositiveText).color());
        group.writeEntry("DecorationFocus", colorSchemes[i].decoration(KColorScheme::FocusColor).color());
        group.writeEntry("DecorationHover", colorSchemes[i].decoration(KColorScheme::HoverColor).color());
    }
    
    KConfigGroup groupWMTheme(config, "WM");
    KConfigGroup groupWMOut(m_config, "WM");
    
    const QStringList colorItemListWM{
        QStringLiteral("activeBackground"),
        QStringLiteral("activeForeground"),
        QStringLiteral("inactiveBackground"),
        QStringLiteral("inactiveForeground"),
        QStringLiteral("activeBlend"),
        QStringLiteral("inactiveBlend")
    };

    const QVector<QColor> defaultWMColors{
        QColor(71,80,87),
        QColor(239,240,241),
        QColor(239,240,241),
        QColor(189,195,199),
        QColor(255,255,255),
        QColor(75,71,67)
    };

    int i = 0;
    for (const QString &coloritem : colorItemListWM) {
        groupWMOut.writeEntry(coloritem, groupWMTheme.readEntry(coloritem, defaultWMColors.value(i)));
        ++i;
    }
    
    const QStringList groupNameList{
        QStringLiteral("ColorEffects:Inactive"),
        QStringLiteral("ColorEffects:Disabled")
    };

    const QStringList effectList{
        QStringLiteral("Enable"),
        QStringLiteral("ChangeSelectionColor"),
        QStringLiteral("IntensityEffect"),
        QStringLiteral("IntensityAmount"),
        QStringLiteral("ColorEffect"),
        QStringLiteral("ColorAmount"),
        QStringLiteral("Color"),
        QStringLiteral("ContrastEffect"),
        QStringLiteral("ContrastAmount")
    };

    for (const QString &groupName : groupNameList) {
        KConfigGroup groupEffectOut(m_config, groupName);
        KConfigGroup groupEffectTheme(config, groupName);

        for (const QString &effect : effectList) {
            groupEffectOut.writeEntry(effect, groupEffectTheme.readEntry(effect));
        }
    }

    m_config->sync();
    
    runRdb(KRdbExportQtColors | KRdbExportGtkTheme | KRdbExportGtkColors );

    //saveGtkColors(config);

    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"),
                                                      QStringLiteral("org.kde.KGlobalSettings"),
                                                      QStringLiteral("notifyChange"));
    message.setArguments({
        0, //previous KGlobalSettings::PaletteChanged. This is now private API in khintsettings
        0  //unused in palette changed but needed for the DBus signature
    });
    QDBusConnection::sessionBus().send(message);
    
    return 0;
}
