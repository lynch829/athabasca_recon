/*
Copyright (C) 2011 Eric Nodwell
enodwell@ucalgary.ca

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BONELAB_MultiThreadedReconstructionRunner_HPP_INCLUDED
#define BONELAB_MultiThreadedReconstructionRunner_HPP_INCLUDED

#include "engines/ReconstructionRunner.hpp"
#include "bonelab/ThreadRunner.hpp"
#include "bonelab/ObjectGroup.hpp"
#include <boost/thread.hpp>


namespace athabasca_recon
  {

  /** A multi-threaded implementation of ReconstructionRunner. */
  template <class TProjectionIn, class TProjectionOut, class TVolume>
  class MultiThreadedReconstructionRunner
    : public ReconstructionRunner<TProjectionIn, TProjectionOut, TVolume>
    {
    public:

      typedef typename TProjectionIn::value_type TProjectionInValue;
      typedef typename TProjectionOut::value_type TProjectionOutValue;
      typedef typename TProjectionOut::space_type TSpace;
      typedef typename TProjectionOut::index_type TIndex;
      typedef CTCalibration<TProjectionIn,TProjectionOut> TCalibration;
      typedef BadPixelCorrector<TProjectionOut> TBadPixelCorrector;
      typedef BeamPowerCorrector<TProjectionOut> TBeamPowerCorrector;
      typedef StreamingWriter<TProjectionOut> TProjectionsWriter;
      typedef typename TVolume::value_type TVolumeValue;
      typedef typename TVolume::index_type TVolumeIndex;
      typedef typename TVolume::space_type TVolumeSpace;
      typedef typename bonelab::Image<2,TVolumeValue,TVolumeIndex,TVolumeSpace> TVolumeSlice;
      typedef StreamingWriter<TVolumeSlice> TVolumeWriter;
      
      MultiThreadedReconstructionRunner();
      ~MultiThreadedReconstructionRunner();
      
      /** Sets the number of worker threads to use.
        * The default is the hardware concurrency (i.e. the number of
        * processing cores).
        */
      void SetNumberOfThreads(int arg) {this->m_NumberOfThreads = arg;}
      
      void Execute();

    protected:
      
      void ExecuteOnThreads(bonelab::ObjectGroup<bonelab::ThreadRunner>& threadRunners);

      int m_NumberOfThreads;
      // Note that these must stay in existence even after synchronization,
      // as threads that have yet to proceed from the synchronization point
      // still require them.
      boost::barrier* m_BarrierMeAndAllWorkers;
      boost::barrier* m_BarrierAllWorkers;
      boost::barrier* m_BarrierMeAndOneWorker;

    };  // class MultiThreadedReconstructionRunner

  } // namespace athabasca_recon

// Template definitions so we don't require explicit instantiations
#include "MultiThreadedReconstructionRunner.txx"

#endif
