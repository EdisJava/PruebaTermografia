TEMPLATE = subdirs

SUBDIRS += \
    Core \
    UI


UI.depends = Core
