#ifndef KIRANSEARCHENTRY_H
#define KIRANSEARCHENTRY_H

#include <gtkmm.h>

class KiranSearchEntry : public Gtk::SearchEntry
{
public:
    KiranSearchEntry();
    friend void on_preedit_text_changed(KiranSearchEntry *entry, char *preedit_text);

protected:
    virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

private:
    bool has_preedit_text;
};

#endif // KIRANSEARCHENTRY_H
