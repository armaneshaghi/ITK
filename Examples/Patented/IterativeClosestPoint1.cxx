/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    IterativeClosestPoint1.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifdef _WIN32
#pragma warning ( disable : 4786 )
#endif

// Software Guide : BeginLatex
//
// This example illustrates how to perform Iterative Closest Point (ICP) 
// registration in ITK. The main class featured in this section is the 
// \doxygen{EuclideanDistancePointMetric}.
//
// Software Guide : EndLatex 



// Software Guide : BeginCodeSnippet
#include "itkTranslationTransform.h"
#include "itkEuclideanDistancePointMetric.h"
#include "itkLevenbergMarquardtOptimizer.h"
#include "itkPointSet.h"
#include "itkPointSetToPointSetRegistrationMethod.h"
// Software Guide : EndCodeSnippet

#include <iostream>
#include <fstream>

class CommandIterationUpdate : public itk::Command 
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

public:

  typedef itk::LevenbergMarquardtOptimizer     OptimizerType;
  typedef const OptimizerType                 *OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event);
  }

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer = 
                         dynamic_cast< OptimizerPointer >( object );

    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }

    std::cout << "Value = " << optimizer->GetCachedValue() << std::endl; 
    std::cout << "Position = "  << optimizer->GetCachedCurrentPosition();
    std::cout << std::endl << std::endl;

  }
   
};


int main(int argc, char * argv[] )
{

  if( argc < 3 )
    {
    std::cerr << "Arguments Missing. " << std::endl;
    std::cerr 
      << "Usage:  IterativeClosestPoint1   fixedPointsFile  movingPointsFile " 
      << std::endl;
    return 1;
    }

// Software Guide : BeginCodeSnippet
  const unsigned int Dimension = 2;

  typedef itk::PointSet< float, Dimension >   PointSetType;

  PointSetType::Pointer fixedPointSet  = PointSetType::New();
  PointSetType::Pointer movingPointSet = PointSetType::New();

  typedef PointSetType::PointType     PointType;

  typedef PointSetType::PointsContainer  PointsContainer;

  PointsContainer::Pointer fixedPointContainer  = PointsContainer::New();
  PointsContainer::Pointer movingPointContainer = PointsContainer::New();

  PointType fixedPoint;
  PointType movingPoint;


  // Read the file containing coordinates of fixed points.
  std::ifstream   fixedFile;
  fixedFile.open( argv[1] );
  if( fixedFile.fail() )
    {
    std::cerr << "Error opening points file with name : " << std::endl;
    std::cerr << argv[1] << std::endl;
    return 2;
    }

  unsigned int pointId = 0;
  fixedFile >> fixedPoint;
  while( !fixedFile.eof() )
    {
    fixedPointContainer->InsertElement( pointId, fixedPoint );
    fixedFile >> fixedPoint;
    pointId++;
    }
  fixedPointSet->SetPoints( fixedPointContainer );
  std::cout << 
    "Number of fixed Points = " << 
    fixedPointSet->GetNumberOfPoints() << std::endl;



  // Read the file containing coordinates of moving points.
  std::ifstream   movingFile;
  movingFile.open( argv[2] );
  if( movingFile.fail() )
    {
    std::cerr << "Error opening points file with name : " << std::endl;
    std::cerr << argv[2] << std::endl;
    return 2;
    }

  pointId = 0;
  movingFile >> movingPoint;
  while( !movingFile.eof() )
    {
    movingPointContainer->InsertElement( pointId, movingPoint );
    movingFile >> movingPoint;
    pointId++;
    }
  movingPointSet->SetPoints( movingPointContainer );
  std::cout << "Number of moving Points = " 
    << movingPointSet->GetNumberOfPoints() << std::endl;


//-----------------------------------------------------------
// Set up  the Metric
//-----------------------------------------------------------
  typedef itk::EuclideanDistancePointMetric<  
                                    PointSetType, 
                                    PointSetType>
                                                    MetricType;

  typedef MetricType::TransformType                 TransformBaseType;
  typedef TransformBaseType::ParametersType         ParametersType;
  typedef TransformBaseType::JacobianType           JacobianType;

  MetricType::Pointer  metric = MetricType::New();


//-----------------------------------------------------------
// Set up a Transform
//-----------------------------------------------------------

  typedef itk::TranslationTransform< double, Dimension >      TransformType;

  TransformType::Pointer transform = TransformType::New();


  // Optimizer Type
  typedef itk::LevenbergMarquardtOptimizer OptimizerType;

  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  optimizer->SetUseCostFunctionGradient(false);

  // Registration Method
  typedef itk::PointSetToPointSetRegistrationMethod< 
                                            PointSetType, 
                                            PointSetType >
                                                    RegistrationType;


  RegistrationType::Pointer   registration  = RegistrationType::New();

  // Scale the translation components of the Transform in the Optimizer
  OptimizerType::ScalesType scales( transform->GetNumberOfParameters() );
  scales.Fill( 0.01 );

  
  unsigned long   numberOfIterations =  100;
  double          gradientTolerance  =  1e-5;    // convergence criterion
  double          valueTolerance     =  1e-5;    // convergence criterion
  double          epsilonFunction    =  1e-6;   // convergence criterion


  optimizer->SetScales( scales );
  optimizer->SetNumberOfIterations( numberOfIterations );
  optimizer->SetValueTolerance( valueTolerance );
  optimizer->SetGradientTolerance( gradientTolerance );
  optimizer->SetEpsilonFunction( epsilonFunction );

  // Start from an Identity transform (in a normal case, the user 
  // can probably provide a better guess than the identity...
  transform->SetIdentity();

  registration->SetInitialTransformParameters( transform->GetParameters() );

  //------------------------------------------------------
  // Connect all the components required for Registration
  //------------------------------------------------------
  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetTransform(     transform     );
  registration->SetFixedPointSet( fixedPointSet );
  registration->SetMovingPointSet(   movingPointSet   );

  // Connect an observer
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );

  try 
    {
    registration->StartRegistration();
    }
  catch( itk::ExceptionObject & e )
    {
    std::cout << e << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Solution = " << transform->GetParameters() << std::endl;




// Software Guide : EndCodeSnippet


  return EXIT_SUCCESS;

}

