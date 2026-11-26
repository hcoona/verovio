/////////////////////////////////////////////////////////////////////////////
// Name:        pedal.cpp
// Author:      Laurent Pugin
// Created:     2016
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#include "pedal.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

#include "doc.h"
#include "functor.h"
#include "horizontalaligner.h"
#include "layerelement.h"
#include "resources.h"
#include "smufl.h"
#include "vrv.h"

//----------------------------------------------------------------------------

#include "MidiFile.h"

namespace vrv {

//----------------------------------------------------------------------------
// Pedal
//----------------------------------------------------------------------------

static const ClassRegistrar<Pedal> s_factory("pedal", PEDAL);

Pedal::Pedal()
    : ControlElement(PEDAL)
    , TimeSpanningInterface()
    , AttExtSymAuth()
    , AttExtSymNames()
    , AttLineRend()
    , AttLineRendBase()
    , AttPedalLog()
    , AttPedalVis()
    , AttPlacementRelStaff()
    , AttVerticalGroup()
{
    this->RegisterInterface(TimeSpanningInterface::GetAttClasses(), TimeSpanningInterface::IsInterface());
    this->RegisterAttClass(ATT_EXTSYMAUTH);
    this->RegisterAttClass(ATT_EXTSYMNAMES);
    this->RegisterAttClass(ATT_LINEREND);
    this->RegisterAttClass(ATT_LINERENDBASE);
    this->RegisterAttClass(ATT_PEDALLOG);
    this->RegisterAttClass(ATT_PEDALVIS);
    this->RegisterAttClass(ATT_PLACEMENTRELSTAFF);
    this->RegisterAttClass(ATT_VERTICALGROUP);

    this->Reset();
}

Pedal::~Pedal() {}

void Pedal::Reset()
{
    ControlElement::Reset();
    TimeSpanningInterface::Reset();
    this->ResetExtSymAuth();
    this->ResetExtSymNames();
    this->ResetLineRend();
    this->ResetLineRendBase();
    this->ResetPedalLog();
    this->ResetPedalVis();
    this->ResetPlacementRelStaff();
    this->ResetVerticalGroup();

    m_endJoint = PedalLineJoint::None;
    m_startJoint = PedalLineJoint::None;
    m_drawingPedalStyle = PEDALSTYLE_NONE;
}

char32_t Pedal::GetPedalGlyph() const
{
    const Resources *resources = this->GetDocResources();
    if (!resources) return 0;

    // If there is glyph.num, prioritize it
    if (this->HasGlyphNum()) {
        char32_t code = this->GetGlyphNum();
        if (resources->GetGlyph(code)) return code;
    }
    // If there is glyph.name (second priority)
    else if (this->HasGlyphName()) {
        char32_t code = resources->GetGlyphCode(this->GetGlyphName());
        if (resources->GetGlyph(code)) return code;
    }

    if ((this->GetDir() == pedalLog_DIR_half) && resources->GetGlyph(SMUFL_E656_keyboardPedalHalf)) {
        return SMUFL_E656_keyboardPedalHalf;
    }

    return (this->GetFunc() == "sostenuto") ? SMUFL_E659_keyboardPedalSost : SMUFL_E650_keyboardPedalPed;
}

int Pedal::GetLineWidth(const Doc *doc, int unit) const
{
    int lineWidth = doc->GetOptions()->m_pedalLineThickness.GetValue() * unit;
    if (this->HasLwidth()) {
        if (this->GetLwidth().GetType() == LINEWIDTHTYPE_lineWidthTerm) {
            switch (this->GetLwidth().GetLineWithTerm()) {
                case LINEWIDTHTERM_narrow: lineWidth *= LINEWIDTHTERM_factor_narrow; break;
                case LINEWIDTHTERM_medium: lineWidth *= LINEWIDTHTERM_factor_medium; break;
                case LINEWIDTHTERM_wide: lineWidth *= LINEWIDTHTERM_factor_wide; break;
                default: break;
            }
        }
        else if (this->GetLwidth().GetType() == LINEWIDTHTYPE_measurementunsigned) {
            if (this->GetLwidth().GetMeasurementunsigned().GetType() == MEASUREMENTTYPE_px) {
                lineWidth = this->GetLwidth().GetMeasurementunsigned().GetPx();
            }
            else {
                lineWidth = this->GetLwidth().GetMeasurementunsigned().GetVu() * unit;
            }
        }
    }
    return lineWidth;
}

//----------------------------------------------------------------------------
// Pedal functor methods
//----------------------------------------------------------------------------

FunctorCode Pedal::Accept(Functor &functor)
{
    return functor.VisitPedal(this);
}

FunctorCode Pedal::Accept(ConstFunctor &functor) const
{
    return functor.VisitPedal(this);
}

FunctorCode Pedal::AcceptEnd(Functor &functor)
{
    return functor.VisitPedalEnd(this);
}

FunctorCode Pedal::AcceptEnd(ConstFunctor &functor) const
{
    return functor.VisitPedalEnd(this);
}

} // namespace vrv
