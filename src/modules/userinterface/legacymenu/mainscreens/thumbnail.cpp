/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "thumbnail.h"
#include <tgfclient.h>
#include <tgf.hpp>
#include <stdexcept>
#include <string>

void thumbnail::pressed() const
{
    GfuiEnable(hscr, btn, GFUI_DISABLE);
    GfuiVisibilitySet(hscr, progress_bar, GFUI_VISIBLE);
    GfuiButtonSetText(hscr, btn, "Downloading");
    cb(args.t, args.args);
}

static void pressed(void *args)
{
    const thumbnail::cbargs *a = static_cast<const thumbnail::cbargs *>(args);

    a->t->pressed();
}

void thumbnail::progress(float p) const
{
    GfuiProgressbarSetValue(hscr, progress_bar, p);
}

thumbnail::thumbnail(void *hscr, void *param, const std::string &id,
    callback cb, void *args) :
    args(this, args),
    img(GfuiMenuCreateStaticImageControl(hscr, param,
        (id + std::string("img")).c_str())),
    name(GfuiMenuCreateLabelControl(hscr, param,
        (id + std::string("name")).c_str())),
    category(GfuiMenuCreateLabelControl(hscr, param,
        (id + std::string("category")).c_str())),
    license(GfuiMenuCreateLabelControl(hscr, param,
        (id + std::string("license")).c_str())),
    author(GfuiMenuCreateLabelControl(hscr, param,
        (id + std::string("author")).c_str())),
    size(GfuiMenuCreateLabelControl(hscr, param,
        (id + std::string("size")).c_str())),
    btn(GfuiMenuCreateButtonControl(hscr, param,
        (id + std::string("btn")).c_str(),
        const_cast<thumbnail::cbargs *>(&this->args), ::pressed)),
    progress_bar(GfuiMenuCreateProgressbarControl(hscr, param,
        (id + std::string("progress")).c_str())),
    cb(cb),
    hscr(hscr)
{
    if (img < 0)
        throw std::runtime_error("img failed");
    else if (name < 0)
        throw std::runtime_error("name failed");
    else if (category < 0)
        throw std::runtime_error("category failed");
    else if (license < 0)
        throw std::runtime_error("license failed");
    else if (author < 0)
        throw std::runtime_error("author failed");
    else if (size < 0)
        throw std::runtime_error("size failed");
    else if (btn < 0)
        throw std::runtime_error("btn failed");
    else if (progress_bar < 0)
        throw std::runtime_error("progress failed");

    clear();
}

void thumbnail::set(const std::string &path, const std::string &name,
    const std::string &category, const std::string &license,
    const std::string &author, const std::string &size) const
{
    GfuiStaticImageSet(hscr, img, path.c_str());
    GfuiLabelSetText(hscr, this->name, name.c_str());
    GfuiLabelSetText(hscr, this->category, category.c_str());
    GfuiLabelSetText(hscr, this->author, author.c_str());
    GfuiLabelSetText(hscr, this->license, license.c_str());
    GfuiLabelSetText(hscr, this->size, size.c_str());
}

void thumbnail::set(const std::string &text, bool enable, bool show,
    float p) const
{
    GfuiEnable(hscr, btn, enable ? GFUI_ENABLE : GFUI_DISABLE);
    GfuiVisibilitySet(hscr, progress_bar, show ? GFUI_VISIBLE : GFUI_INVISIBLE);
    GfuiButtonSetText(hscr, btn, text.c_str());
    progress(p);
}

void thumbnail::clear() const
{
    GfuiStaticImageSet(hscr, img, "data/img/notready.png");
    GfuiLabelSetText(hscr, name, "");
    GfuiLabelSetText(hscr, category, "");
    GfuiLabelSetText(hscr, author, "");
    GfuiLabelSetText(hscr, license, "");
    GfuiLabelSetText(hscr, size, "");
    set();
}
