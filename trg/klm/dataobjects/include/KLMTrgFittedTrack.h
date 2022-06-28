/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Dmitri Liventsev                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMTrgFittedTrack_H
#define KLMTrgFittedTrack_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {
  //! Store KLM TRG track information as a ROOT object
  class KLMTrgFittedTrack : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    KLMTrgFittedTrack() = default;




    //! Destructor
    virtual ~KLMTrgFittedTrack() { }

    // accessors

    double getslopeXY() const
    {
      return slopeXY;
    }
    double getInterceptXY() const
    {
      return interceptXY;
    }
    double getIpXY() const
    {
      return ipXY;
    }
    double getChisqXY() const
    {
      return chisqXY;
    }


    void setslopeXY(double slopeXY_)
    {
      slopeXY = slopeXY_;
    }
    void setInterceptXY(double InterceptXY_)
    {
      interceptXY = InterceptXY_;
    }
    void setIpXY(double ipXY_)
    {
      ipXY = ipXY_;
    }
    void setChisqXY(double chisqXY_)
    {
      chisqXY = chisqXY_;
    }

    int getSubdetector() const
    {
      return Subdetector;
    }
    int getSection() const
    {
      return Section;
    }
    int getSector() const
    {
      return Sector ;
    }
    int getPlane() const
    {
      return Plane;
    }

    void setSubdetector(int Subdetector_)
    {
      Subdetector = Subdetector_;
    }
    void setSection(int Section_)
    {
      Section = Section_;
    }
    void setSector(int Sector_)
    {
      Sector = Sector_;
    }
    void setPlane(int Plane_)
    {
      Plane = Plane_;
    }
  private:
    double slopeXY = 0, interceptXY = 0, ipXY = 0, chisqXY = 0;
    int Subdetector = 0, Section = 0, Sector = 0, Plane = 0;

    ClassDef(KLMTrgFittedTrack, 2);
  };
} // end of namespace Belle2

#endif //KLMTRACKFITTER_H