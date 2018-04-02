/***************************************************************************
 *
 * Author: "Jasenko Zivanov & Sjors H.W. Scheres"
 * MRC Laboratory of Molecular Biology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * This complete copyright notice must be included in any revised version of the
 * source code. Additional authorship citations may be added, but existing
 * author citations must be preserved.
 ***************************************************************************/

#ifndef MOTION_REFINER_H_
#define MOTION_REFINER_H_


#include <src/ctf.h>
#include <src/image.h>
#include <src/fftw.h>
#include <src/backprojector.h>
#include <src/micrograph_model.h>
#include <src/jaz/obs_model.h>
#include <src/jaz/gravis/t2Vector.h>
#include <src/jaz/parallel_ft.h>

#include <src/jaz/micrograph_handler.h>
#include <src/jaz/reference_map.h>

#include "motion_param_estimator.h"
#include "motion_estimator.h"
#include "frame_recombiner.h"

#include <omp.h>


class MotionRefiner
{
    public:

        MotionRefiner();

            // Verbosity
            int verb;

            // Write out debugging information
            bool debug;

            double movie_angpix, coords_angpix, angpix;
            int nr_omp_threads, firstFrame, lastFrame;
            std::string outPath; // move to micrographHandler? - Yes!


            ObservationModel obsModel;

            ReferenceMap reference;
            MotionParamEstimator motionParamEstimator;
            MotionEstimator motionEstimator;
            FrameRecombiner frameRecombiner;


            // Q: Jasenko, can we have more informative names for these important variables?
            // A: They are so important and common that their names should be short!
            // (s: full image size, sh: half-size + 1, fc: frame count
            //  - these are consistent throughout the codebase.)
            int s, sh, fc;

            int micrograph_xsize, micrograph_ysize;


        // Read command line arguments
        void read(int argc, char **argv);

        // Initialise some general stuff after reading
        void init();

        // General Running (Admiral Swimming!)
        void run();

        double angToPix(double a);
        double pixToAng(double p);

        // For original particle-polishing-like Bfactors (not used)
        void calculateSingleFrameReconstruction(int iframe);

        // Get output STAR file name for this micrograph
        FileName getOutputFileNameRoot(const MetaDataTable& mdt);

        // load a movie and extract all particles
        // returns a per-particle vector of per-frame images of size sh x s
        std::vector<std::vector<Image<Complex>>> loadMovie(
                const MetaDataTable& mdt, std::vector<ParFourierTransformer>& fts);

        // does the same and then also loads the particle tracks for particles at positions pos
        std::vector<std::vector<Image<Complex>>> loadMovie(
                const MetaDataTable& mdt, std::vector<ParFourierTransformer>& fts,
                const std::vector<gravis::d2Vector>& pos,
                std::vector<std::vector<gravis::d2Vector>>& tracks,
                bool unregGlob, std::vector<gravis::d2Vector>& globComp);

        bool hasCorrMic();

    protected:

            std::string
                starFn, movie_toReplace, movie_replaceBy;

            // Allow continuation of crashed jobs
            bool only_do_unfinished;

            bool estimateParams,
                 estimateMotion,
                 recombineFrames,
                 generateStar;

            long maxMG, minMG;


            MetaDataTable mdt0;
            std::vector<MetaDataTable>
                allMdts, // all micrographs (used for B-factor computation)
                chosenMdts, // micrographs between minMG and maxMG
                paramMdts, // micrographs on which the parameters are to be estimated
                motionMdts, recombMdts; // unfinished micrographs


            MicrographHandler micrographHandler;


        // load the header of the first movie only to learn the frame number and micrograph size
        // (also the dose and movie pixel size if a corrected_micrographs.star is available)
        void loadInitialMovie();

        // combine all EPS files into one logfile.pdf
        void combineEPSAndSTARfiles();

        // apply changes to micrograph-filenames implied by
        // movie_path, movie_ending and movie_toReplace/replaceBy
        void adaptMovieNames();
};



#endif
