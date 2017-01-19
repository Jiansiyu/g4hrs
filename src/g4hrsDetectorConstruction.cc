

#include "g4hrsDetectorConstruction.hh"
#include "g4hrsGenericDetector.hh"
#include "g4hrsBeamTarget.hh"
#include "g4hrsGlobalField.hh"
#include "g4hrsRun.hh"
#include "g4hrsRunData.hh"
#include "g4hrsIO.hh"


#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4RunManager.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4NistManager.hh"

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4UserLimits.hh"
#include "globals.hh"

#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"

#include "G4UImanager.hh"
#include "G4UIcommand.hh"

#include "G4ios.hh"

#include "G4UnitsTable.hh"


//visual
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#define __DET_STRLEN 200
#define __MAX_DETS 5000

g4hrsDetectorConstruction::g4hrsDetectorConstruction() {
    // Default geometry file
    CreateGlobalMagneticField();

    fWorldVolume = NULL;
}

g4hrsDetectorConstruction::~g4hrsDetectorConstruction() {
}

G4VPhysicalVolume* g4hrsDetectorConstruction::Construct() {
    G4VPhysicalVolume *worldVolume;



  return fWorldVolume;
}

g4hrsDetectorConstruction::CreateTarget(G4LogicalVolume *pMotherLogVol){

    G4VSolid* targetSolid  = new G4Box("targetBox", mTargetW / 2.0, mTargetH / 2.0, mTargetL / 2.0 );
    G4LogicalVolume* targetLogical = new G4LogicalVolume(targetSolid,mMaterialManager->lead208,"targetLogical",0,0,0);
    new G4PVPlacement(0,G4ThreeVector(pTg2SC_X,-pTg2SC_Z,pTg2SC_Y - 0.5 * ( mTargetL + diamondL )),
            //caseenLogical,"caseenPhys",targetContainerLogical,0,0);
        caseenLogical,"caseenPhys",pMotherLogVol,0,0);

    beamtarg->AddVolume(targetLogical);
}

g4hrsDetectorConstruction::CreateSeptum(G4LogicalVolume *pMotherLogVol){
	const double inch=2.54*cm;
	double startphi,deltaphi;
	G4String SDname;

	// All G4VSensitiveDetector will be managed (deleted) by SDManager
	//therefore no need to delete it at the end of this subroutine
	G4VSensitiveDetector* sieveSlitSD=new HRSStdSD(SDname="sieveSlit");
	G4VSensitiveDetector* septumWindowSD=new HRSStdSD(SDname="septumWindow");

	G4SDManager* SDman = G4SDManager::GetSDMpointer();

	int pSeptum_UseUniformB=0;
	gConfig->GetParameter("Septum_UseUniformB",pSeptum_UseUniformB); 
	double pSeptumCurrentRatioL=1.0, pSeptumCurrentRatioR=1.0;  
	gConfig->GetParameter("Septum_CurrentRatioL",pSeptumCurrentRatioL);
	gConfig->GetParameter("Septum_CurrentRatioR",pSeptumCurrentRatioR);
	int pUseSeptumField=(pSeptum_UseUniformB==0 && 
		(fabs(pSeptumCurrentRatioL)>1.0E-08 || fabs(pSeptumCurrentRatioR)>1.0E-08) )?1:0;
	//set up the septum only if there is an angle difference	
	bool mSetupSeptumBlock=((mLHRSAngle-mLSeptumAngle)/deg>0.5)?true:false;
	//cout << mLHRSAngle-mLSeptumAngle << " = " << mLHRSAngle << " - " << mLSeptumAngle << " divided by " << deg << endl;
	//cout << mRHRSAngle-mRSeptumAngle << " = " << mRHRSAngle << " - " << mRSeptumAngle << " divided by " << deg << endl;

	//cout << "True or false? " << mSetupSeptumBlock << endl;
	/////////////////////////////////////////////////
	//From Hall A NIM Paper, the standard sieve slit
	//Each spectrometer is equipped with a set of collimators, positioned 1:109 +/- 0.005 
	//and 1:101+/-0.005 m from the target on the left and right spectrometers, respectively. 
	//There is a large collimator, made of 80 mm thick tungsten, with a 121.8 mm vertical 
	//opening and a 62:9 mm horizontal opening at the entrance face. The opening in this 
	//collimator expands to 129.7 by 66.8 mm at the exit face. A second smaller collimator, 
	//made of the same material, is 50.0 by 21.3 mm at the entrance face and 53.2 by 22:6 mm 
	//at the exit face. The third collimator is the sieve slit, which is used to study the 
	//optical properties of the spectro- meters. The sieve is a 5 mm thick stainless steel 
	//sheet with a pattern of 49 holes 7 x 7, spaced 25 mm apart vertically and 12:5 mm apart 
	//horizontally. Two of the holes, one in the center and one displaced two rows vertically 
	//and one horizontally, are 4 mm in diameter. The rest of the holes are 2 mm in diameter. 
	//The sieve slits are positioned 75 mm further from the target than the other collimators.

	//double mLHRSAngle=12.5*deg,mRHRSAngle=-12.5*deg;
	//double mLSeptumAngle=5.69*deg,mRSeptumAngle=-5.69*deg;
	G4RotationMatrix *pRotLHRS=new G4RotationMatrix();
	pRotLHRS->rotateY(-mLHRSAngle); 
	G4RotationMatrix *pRotRHRS=new G4RotationMatrix();
	pRotRHRS->rotateY(-mRHRSAngle); 
	G4RotationMatrix *pRotLSeptum=new G4RotationMatrix();
	pRotLSeptum->rotateY(-mLSeptumAngle); 
	G4RotationMatrix *pRotRSeptum=new G4RotationMatrix();
	pRotRSeptum->rotateY(-mRSeptumAngle); 
	G4RotationMatrix *pRotNone=new G4RotationMatrix();
	pRotNone->rotateY(0); 

	G4RotationMatrix *pRotX90deg=new G4RotationMatrix();
	pRotX90deg->rotateX(-90*deg); 
	/////////////////////////////////////////////////
	G4VPhysicalVolume* theG2PSeptumPhys=0;

	///////////////////////////////////////
	//Sieve Slit for HRS-Angle=5.65
	///////////////////////////////////////

	////////////////the following is for 6 degree sieve//////////////

	//these 2 will be read from ini file, it is 80.0cm for 6 deg and 120 cm for 12.5 deg
	//double mPivot2LSieveFace=79.96*cm;
	//double mPivot2RSieveFace=79.96*cm;

	double pSieveSlitX=2.205*inch; //33.13*mm
	double pSieveSlitY=5.134*inch; //130.40*mm
	double pSieveSlitZ=0.2*inch;

	double pSieveSlitHoleR=0.6985*mm;           //radius of small hole 0.055/2 inch
	double pSieveSlitLargeHoleR=1.3462*mm;      //radius of large hole 0.106/2 inch
	double pSieveSlitHoldL=pSieveSlitZ+0.1*mm;  //need to make it longer to avoid round off in the subtraction

	//the big center hole horizontal and vertical offset, From servey
	double pSieveSlitLargeHoleH=0*mm;    //positive means shift away from the beam line
	double pSieveSlitLargeHoleV=0*mm;

	//please note that the following constants are for the sieve in the right arm only
	//need to mirror(flip) it in order to put in the left arm  
	double pSieveSlitDeltaH[8]={0.537*inch, 0.188*inch, 0.188*inch, 0.188*inch,
		0.241*inch, 0.241*inch, 0.241*inch, 0.381*inch}; //in inch, from left to right
	double pSieveSlitDeltaV[8]={0.496*inch, 0.524*inch, 0.524*inch, 0.524*inch,
		0.524*inch, 0.524*inch, 0.524*inch, 1.494*inch}; //in inch, from top to buttom

	//the whole position relative to the slit center 
	double pSieveSlitHolePosH[7], pSieveSlitHolePosV[7];
	for(int ii=0;ii<7;ii++)
	{
		pSieveSlitHolePosH[ii] = (ii==0)?pSieveSlitX/2.0:pSieveSlitHolePosH[ii-1];
		pSieveSlitHolePosH[ii] -= pSieveSlitDeltaH[ii];

		pSieveSlitHolePosV[ii] = (ii==0)?pSieveSlitY/2.0:pSieveSlitHolePosV[ii-1];
		pSieveSlitHolePosV[ii] -= pSieveSlitDeltaV[ii];
	}

	////////////////the following is for 12.5 degree sieve//////////////	
	
	if(!mSetupSeptumBlock)
	{
		//these 2 will be read from ini file, it is 80.0cm for 6 deg and 120 cm for 12.5 deg
		//mPivot2LSieveFace=115.6*cm;
		//mPivot2RSieveFace=116.39*cm;

		pSieveSlitX=5.687*inch; //144.45*mm
		pSieveSlitY=7.750*inch; //196.85*mm
		pSieveSlitZ=0.25*inch;

		pSieveSlitHoleR=1.0033*mm;           //radius of small hole 0.079/2 inch
		pSieveSlitLargeHoleR=1.994*mm;       //radius of large hole 0.157/2 inch
		pSieveSlitHoldL=pSieveSlitZ+0.1*mm;  //need to make it longer to avoid round off in the subtraction

		double pFirstHole2Top=0.799*inch, pFirstHole2Left=1.306*inch;
		double pHoleSpanH=0.513*inch, pHoleSpanV=1.025*inch;
		for(int ii=0;ii<7;ii++)
		{
			pSieveSlitHolePosH[ii] = (ii==0)?pSieveSlitX/2.0-pFirstHole2Left:pSieveSlitHolePosH[ii-1]-pHoleSpanH;
			pSieveSlitHolePosV[ii] = (ii==0)?pSieveSlitY/2.0-pFirstHole2Top:pSieveSlitHolePosV[ii-1]-pHoleSpanV;
		}

		//the big center hole horizontal and vertical offset, From Alan
		pSieveSlitLargeHoleH=0.0;
		pSieveSlitLargeHoleV=0.0;
	}


	//now start to build box then subtract 49 holes 
	G4VSolid* sieveSlitWholeSolid=new G4Box("sieveSlitWholeBox",pSieveSlitX/2.0,
		pSieveSlitY/2.0,pSieveSlitZ/2.0);
	startphi=0.0*deg;deltaphi=360.0*deg;
	G4VSolid* sieveSlitHoleSolid=new G4Tubs("sieveSlitHoleTubs",0,pSieveSlitHoleR,
		pSieveSlitHoldL/2.0,startphi,deltaphi); 
	G4VSolid* sieveSlitLargeHoleSolid=new G4Tubs("sieveSlitLargeHoleTubs",0,
		pSieveSlitLargeHoleR,pSieveSlitHoldL/2.0,startphi,deltaphi); 
	G4SubtractionSolid* sieveSlitSolid=(G4SubtractionSolid*)sieveSlitWholeSolid;
	char strName[100];
	for(int ih=0;ih<7;ih++)
	{
		for(int iv=0;iv<7;iv++)
		{
			sprintf(strName,"sieveSlitHole_H%d_V%d",ih,iv);
			if((ih==3 && iv==3) || (ih==4 && iv==1)) 
			{
				//now dig large holes in the block
				sieveSlitSolid=new G4SubtractionSolid(strName,sieveSlitSolid,
					sieveSlitLargeHoleSolid,0,
					G4ThreeVector(pSieveSlitHolePosH[ih],pSieveSlitHolePosV[iv],0));
			}
			else
			{
				//now dig small holes in the block
				sieveSlitSolid=new G4SubtractionSolid(strName,sieveSlitSolid,
					sieveSlitHoleSolid,0,
					G4ThreeVector(pSieveSlitHolePosH[ih],pSieveSlitHolePosV[iv],0));
			}
		}
	}
	sieveSlitSolid->SetName("sieveSlitSolid");

	G4LogicalVolume* sieveSlitLogical = new G4LogicalVolume(sieveSlitSolid,
		mMaterialManager->tungsten,"sieveSlitLogical",0,0,0);
	sieveSlitLogical->SetVisAttributes(LeadVisAtt); 

	SDman->AddNewDetector(sieveSlitSD);
	sieveSlitLogical->SetSensitiveDetector(sieveSlitSD);


	//calculate the center position in the Lab frame
	double pSieveSlitCenterHOffset=pSieveSlitLargeHoleH-pSieveSlitHolePosH[3];
	double pSieveSlitCenterVOffset=pSieveSlitLargeHoleV-pSieveSlitHolePosV[3];


	//place the sieve slits in the hall
	double pLSieveSlitPos_X=(mPivot2LSieveFace+pSieveSlitZ/2.0)*sin(mLSeptumAngle)+
		pSieveSlitCenterHOffset+mPivotXOffset;
	double pLSieveSlitPos_Y=pSieveSlitCenterVOffset+mPivotYOffset;
	double pLSieveSlitPos_Z=(mPivot2LSieveFace+pSieveSlitZ/2.0)*cos(mLSeptumAngle)+mPivotZOffset;
	double pRSieveSlitPos_X=(mPivot2RSieveFace+pSieveSlitZ/2.0)*sin(mRSeptumAngle)-
		pSieveSlitCenterHOffset+mPivotXOffset;
	double pRSieveSlitPos_Y=pSieveSlitCenterVOffset+mPivotYOffset;
	double pRSieveSlitPos_Z=(mPivot2RSieveFace+pSieveSlitZ/2.0)*cos(mRSeptumAngle)+mPivotZOffset;

	if(mSetupLSieveSlit)
	{ 
		G4RotationMatrix *pRotLSieve=new G4RotationMatrix();
		pRotLSieve->rotateY(-mLSeptumAngle-180*deg);
		new G4PVPlacement(pRotLSieve,
		G4ThreeVector(pLSieveSlitPos_X,pLSieveSlitPos_Y,pLSieveSlitPos_Z),
		sieveSlitLogical,"leftSieveSlitPhys",motherLogical,0,0);
	}
	if(mSetupRSieveSlit)
	{
	  new G4PVPlacement(pRotRSeptum,
	  G4ThreeVector(pRSieveSlitPos_X,pRSieveSlitPos_Y,pRSieveSlitPos_Z),
	  sieveSlitLogical,"rightSieveSlitPhys",motherLogical,0,0);
	}

	/////////////////////////
	// Septum block 
	/////////////////////////
	//by Jixie: Allow one to setup septum without HRS
	double col_distance = 1.38*m;//or is it 1.39?
	if(mSetupSeptumBlock)
	{
		////////////////////////////////////////////////////////////////////
		//Septum block, 140 cm width, 84.4 cm height and 74 cm in length, silicon steel
		//Tunnel size: started from x=8.4cm, 30.4cm wide and 24.4 cm in height
		//located at z=700 mm, no rotation
		double pSeptumX=140.0*cm;
		double pSeptumY=84.4*cm;
		double pSeptumZ=74.0*cm;
		double pSeptumTunnelX=30.4*cm;
		double pSeptumTunnelY=24.4*cm;//-2.0*inch;  //By Jixie @20120205: Add 2 inches of iron
		double pSeptumBeamHoleX=7.8*cm;
		double pSeptumBeamHoleY=8.0*cm;



		double pSeptumTunnelPos_X=8.4*cm+pSeptumTunnelX/2.0;
		double pSeptumPos_Z=70.0*cm;		
		gConfig->GetParameter("Septum_OriginZ",pSeptumPos_Z); 
		pSeptumPos_Z*=cm;

		G4VSolid* septumBlockSolid = new G4Box("septumBlockBox",pSeptumX/2.0,
			pSeptumY/2.0,pSeptumZ/2.0);

		//Left and right tunnels, treat the cu coils as part of the block
		//By Jixie: I reduced this by 0.2cm for the Helium bag
		G4VSolid* septumTunnelSolid = new G4Box("septumTunnelBox",pSeptumTunnelX/2.0-0.2*cm,
			pSeptumTunnelY/2.0-0.2*cm,pSeptumZ/2.0+1.0*mm);

		//beam pine hole
		G4VSolid* septumBeamHoleSolid = new G4Box("septumBeamHoleBox",pSeptumBeamHoleX/2.0,
			pSeptumBeamHoleY/2.0,pSeptumZ/2.0+1.0*mm);

		//dig 3 holes, left, right tunnel and beam hole
		G4SubtractionSolid* septumBlockSubLSolid=new G4SubtractionSolid("septumBlockSubL",
			septumBlockSolid,septumTunnelSolid,0,G4ThreeVector(pSeptumTunnelPos_X,0,0));
		G4SubtractionSolid* septumBlockSubLRSolid=new G4SubtractionSolid("septumBlockSubLR",
			septumBlockSubLSolid,septumTunnelSolid,0,G4ThreeVector(-pSeptumTunnelPos_X,0,0));
		G4SubtractionSolid* septumBlockSubLRCSolid=new G4SubtractionSolid("septumBlockSubLRC",
			septumBlockSubLRSolid,septumBeamHoleSolid);

		G4LogicalVolume* septumLogical = new G4LogicalVolume(septumBlockSubLRCSolid,
			mMaterialManager->siliconsteel,"septumLogical",0,0,0);
		septumLogical->SetVisAttributes(IronVisAtt);


		G4int placeseptum = 1;
		//put it in the hall, no rotation
		if( placeseptum ){
		  new G4PVPlacement(0,G4ThreeVector(0,0,pSeptumPos_Z),
				    septumLogical,"septumPhys",motherLogical,0,0,0);
		}

		double pSeptumCoilRadius = 22.06*cm;
		double pSeptumCoilCenterX = 2.83*cm;
		double pSeptumCoilCenterY = -10.25*cm;
		double pSeptumCoilThickness = 4.5*cm;
		G4VSolid* septumCoilCylinderSolid = new G4Tubs("septumCoilCylinderTub",
			0,pSeptumCoilRadius,pSeptumCoilThickness/2.0,0,360*deg);
	  //8
		double septumCoilRecX = 25.0*cm;
		double septumCoilRecY = 15.0*cm;
		G4VSolid* septumCoilRecSolid = new G4Box("septumCoilRecBox",
			septumCoilRecX/2.0,septumCoilRecY/2.0,pSeptumCoilThickness/2.0);

		G4IntersectionSolid* septumCoilSolid = new G4IntersectionSolid("septumCoilSolid",
			septumCoilCylinderSolid,septumCoilRecSolid,0,
			G4ThreeVector(-pSeptumCoilCenterX-septumCoilRecX/2.0,-pSeptumCoilCenterY+septumCoilRecY/2.0,0));

		G4LogicalVolume* septumCoilLogical = new G4LogicalVolume(septumCoilSolid,
			mMaterialManager->copper,"septumCoilLogical",0,0,0);
		septumCoilLogical->SetVisAttributes(CuBrownVisAtt);

		//place 16 copies into the septum container
		double pSeptumCoilPos_X_in   = pSeptumTunnelPos_X-pSeptumTunnelX/2.0-pSeptumCoilThickness/2.0;
		double pSeptumCoilPos_X_out  = pSeptumTunnelPos_X+pSeptumTunnelX/2.0+pSeptumCoilThickness/2.0;
		double pSeptumCoilPos_Y      = pSeptumCoilCenterY-pSeptumTunnelY/2.0;
		double pSeptumCoilPos_Z_up   = pSeptumPos_Z-pSeptumZ/2.0+pSeptumCoilCenterX;
		double pSeptumCoilPos_Z_down = pSeptumPos_Z+pSeptumZ/2.0-pSeptumCoilCenterX;


		//place up stream coils in the following order (looking downstream)
		//#####2######1###7######6#####
		//######      |###|      |#####
		//######      |###|      |#####
		//#####3######0###4######5#####
		G4RotationMatrix* pSeptumCoilRotFrontDown = new G4RotationMatrix();
		pSeptumCoilRotFrontDown->rotateY(90*deg);
		G4RotationMatrix* pSeptumCoilRotFrontUp = new G4RotationMatrix();
		pSeptumCoilRotFrontUp->rotateY(90*deg);
		pSeptumCoilRotFrontUp->rotateX(180*deg);

		if( placeseptum ){
		new G4PVPlacement(pSeptumCoilRotFrontDown,
			G4ThreeVector(pSeptumCoilPos_X_in,pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,0,0);
		new G4PVPlacement(pSeptumCoilRotFrontUp,
			G4ThreeVector(pSeptumCoilPos_X_in,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,1,0);
		new G4PVPlacement(pSeptumCoilRotFrontUp,
			G4ThreeVector(pSeptumCoilPos_X_out,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,2,0);
		new G4PVPlacement(pSeptumCoilRotFrontDown,
			G4ThreeVector(pSeptumCoilPos_X_out,pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,3,0);

		new G4PVPlacement(pSeptumCoilRotFrontDown,
			G4ThreeVector(-pSeptumCoilPos_X_out,pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,4,0);
		new G4PVPlacement(pSeptumCoilRotFrontUp,
			G4ThreeVector(-pSeptumCoilPos_X_out,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,5,0);
		new G4PVPlacement(pSeptumCoilRotFrontUp,
			G4ThreeVector(-pSeptumCoilPos_X_in,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,6,0);
		new G4PVPlacement(pSeptumCoilRotFrontDown,
			G4ThreeVector(-pSeptumCoilPos_X_in,pSeptumCoilPos_Y,pSeptumCoilPos_Z_up),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,7,0);
		}

		//place down stream coils in the following order (looking downstream)
		//####10######9###15#####14####
		//######      |###|      |#####
		//######      |###|      |#####
		//####11######8###12#####13####
		G4RotationMatrix* pSeptumCoilRotBackDown = new G4RotationMatrix();
		pSeptumCoilRotBackDown->rotateY(270*deg);
		G4RotationMatrix* pSeptumCoilRotBackUp = new G4RotationMatrix();
		pSeptumCoilRotBackUp->rotateY(270*deg);
		pSeptumCoilRotBackUp->rotateX(180*deg);

		if( placeseptum ){
		new G4PVPlacement(pSeptumCoilRotBackDown,
			G4ThreeVector(pSeptumCoilPos_X_in,pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,8,0);
		new G4PVPlacement(pSeptumCoilRotBackUp,
			G4ThreeVector(pSeptumCoilPos_X_in,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,9,0);
		new G4PVPlacement(pSeptumCoilRotBackUp,
			G4ThreeVector(pSeptumCoilPos_X_out,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,10,0);
		new G4PVPlacement(pSeptumCoilRotBackDown,
			G4ThreeVector(pSeptumCoilPos_X_out,pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,11,0);

		new G4PVPlacement(pSeptumCoilRotBackDown,
			G4ThreeVector(-pSeptumCoilPos_X_out,pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,12,0);
		new G4PVPlacement(pSeptumCoilRotBackUp,
			G4ThreeVector(-pSeptumCoilPos_X_out,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,13,0);
		new G4PVPlacement(pSeptumCoilRotBackUp,
			G4ThreeVector(-pSeptumCoilPos_X_in,-pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,14,0);
		new G4PVPlacement(pSeptumCoilRotBackDown,
			G4ThreeVector(-pSeptumCoilPos_X_in,pSeptumCoilPos_Y,pSeptumCoilPos_Z_down),
			septumCoilLogical,"septumCoilPhys",motherLogical,true,15,0);
		}
	}
	
	/////////////////////////
	// HRS Virtual Boundary, 
	/////////////////////////
	//For 6 degrees, if septum field is valid and argument UseSeptumPlusStdHRS==1, will 
	//place the virtual boundary 1 cm before HRSContainer (pHRSContainerRin-6*cm = 1.40m)
	//otherwise will place the VB at the position given by the Detector.ini
	//For 12.5 degrees, always place VB 1.40m away from the hall center

	int pUseSeptumPlusStdHRS=0;
	gConfig->GetArgument("UseSeptumPlusStdHRS",pUseSeptumPlusStdHRS);
	//bool nickietest = false;
	int    mSnakeModel;
	gConfig->GetArgument("SnakeModel",mSnakeModel);
	//if( mSnakeModel == 49){
	if( mSnakeModel == 49494949){
	  //do nothing
	}else if( ( (pUseSeptumField && pUseSeptumPlusStdHRS) || !mSetupSeptumBlock ) )//&& ( nickietest ) ) 
	//if(0)
	{
	  cout << "THIS IS NICKIE'S TEST: " << pUseSeptumField << " " << pUseSeptumPlusStdHRS << " " << mSetupSeptumBlock << endl;
	  //bool nickietest = ((pUseSeptumField && pUseSeptumPlusStdHRS) || !mSetupSeptumBlock);
	  //cout << nickietest << endl;
		//this part is trying to place a virtual boundary 1 cm in front of the HRSContainer
		//It is for the case that we use the septum field to propogate electrons to Q1 entrance 
		//and then use the STD HRS transportation other than use 5.69 degrees HRS transportation

		//Place both left and right VB for HRS, which is pHRSContainerRin-6.0*cm away from the 
		//hall center(1.40m). This aperture is a round disk of 30 cm diameter
		//The real Q1 vacumn entrance to hall center is 1.312m, 
	  //G4VSolid* HRSVBSolid = new G4Tubs("HRSVBTub",0.0,15*cm,
	  G4VSolid* HRSVBSolid = new G4Tubs("HRSVBTub",0.0,20*cm,//I am enlarging the size of the disk (Nickie)
			mHRSVBThick/2.0,0.0,360.0*deg);
		G4LogicalVolume* HRSVBLogical = new G4LogicalVolume(HRSVBSolid,
			mMaterialManager->mylar,"HRSVBLogical",0,0,0);
		SDman->AddNewDetector(septumWindowSD);
		HRSVBLogical->SetSensitiveDetector(septumWindowSD);
		HRSVBLogical->SetVisAttributes(LightYellowVisAtt); 



		G4VSolid* TargetSolid = new G4Tubs("TargetTub",0.0,50*cm,
			mHRSVBThick/2.0,0.0,360.0*deg);
		G4LogicalVolume* TargetLogical = new G4LogicalVolume(TargetSolid,
			mMaterialManager->mylar,"TargetLogical",0,0,0);
		SDman->AddNewDetector(septumWindowSD);
		TargetLogical->SetSensitiveDetector(septumWindowSD);
		TargetLogical->SetVisAttributes(LightYellowVisAtt); 

		//double pHallCenter2VB=1.40*m;

		//double pHallCenter2VB=1.34717*m;//This was activated orginally.
		double pHallCenter2VB=col_distance;//This is Nickie's correction: 30 cm from Q1 entrance
		//double pHallCenter2VB=1.24717*m;
		gConfig->SetParameter("Pivot2LHRSVBFace",pHallCenter2VB-mPivotZOffset*cos(mLHRSAngle));
		gConfig->SetParameter("Pivot2RHRSVBFace",pHallCenter2VB-mPivotZOffset*cos(mRHRSAngle)); 

		if(mSetupLHRS)
		{
			double pLHRSVBPos_X=(pHallCenter2VB-mHRSVBThick/2)*sin(mLHRSAngle)+mPivotXOffset;
			double pLHRSVBPos_Y=mPivotYOffset;
			//no need to correct for pivot since the distance is from the hall center
			double pLHRSVBPos_Z=(pHallCenter2VB-mHRSVBThick/2.0)*cos(mLHRSAngle);
			new G4PVPlacement(pRotLHRS,G4ThreeVector(pLHRSVBPos_X,pLHRSVBPos_Y,pLHRSVBPos_Z),
				HRSVBLogical,"virtualBoundaryPhys_LHRS_vb",motherLogical,0,0,0);
		}
		if(mSetupRHRS)
		{
			double pRHRSVBPos_X=(pHallCenter2VB-mHRSVBThick/2)*sin(mRHRSAngle)+mPivotXOffset;
			double pRHRSVBPos_Y=mPivotYOffset;
			//no need to correct for pivot since the distance is from the hall center
			double pRHRSVBPos_Z=(pHallCenter2VB-mHRSVBThick/2)*cos(mRHRSAngle); 
			new G4PVPlacement(pRotRHRS,G4ThreeVector(pRHRSVBPos_X,pRHRSVBPos_Y,pRHRSVBPos_Z),
				HRSVBLogical,"virtualBoundaryPhys_RHRS_vb",motherLogical,0,0,0);
		}
		//cout << "Snake model " << mSnakeModel << endl;
		//cout << "Snake model " << mSnakeModel << endl;
		//cout << "Snake model " << mSnakeModel << endl;
		//cout << "Snake model " << mSnakeModel << endl;


		//if(mSnakeModel==47){
		//double pCHRSVBPos_X=mPivotXOffset;// + mHRSVBThick/2.0 * cos(mLSeptumAngle);
		// double pCHRSVBPos_Y=mPivotYOffset;
		  //no need to correct for pivot since thdistance is from the hall center
		  //double pCHRSVBPos_Z=(mHRSVBThick/2.0 - 110 * cm);
		  //double pCHRSVBPos_Z=( - 100 * cm );//DANGER IN PLACEMENT
		  //new G4PVPlacement(pRotNone,G4ThreeVector(pCHRSVBPos_X,pCHRSVBPos_Y,pCHRSVBPos_Z),
		  //TargetLogical,"virtualBoundaryPhys_C",motherLogical,0,0,0);
		  //cout << "The angle of the HRS is: " << -mLHRSAngle << endl;
		  //}
	}else if( mSnakeModel == 49 || mSnakeModel == 48 || mSnakeModel == 51 || mSnakeModel == 53){

	  /////////////////////////
	  // HRS Q1              // Nickie is also adding the Q1 collimator here
	  /////////////////////////
	  double pQ1Rin=15.0*cm;
	  //double pQ1Length=(1.698M-1.36*m)*2+0.8*m;

	  //Nickie puts in Paul's collimator////////////////////////////////////////////////////////////////
	  //Paul's collimator///////////////////////////////////////////////////////////////////////////////
	  double PaulColT  = 0.01 * m; //This is obviously just a placeholder.
	  double buffer2   = 4.5 * cm;
	  double box2h     = 11.7 * cm - 20 * cm * sin(acos(18.9/20.0));
	  double box2w     = 4.0 * cm;
	  double box3h     = 2.0 * cm;
	  double box3w     = 10.0 * cm;
	  //double box3h     = 10.0 * cm;
	  //double box3w     = 20.0 * cm;
	  double box3x     = 4.0 * cm;
	  double box3y     = -1.88 * box3x + 14.74 * cm;
	  double box3deg   = 28.0 * deg;

	  double pHallCenter2Col=col_distance;//This is Nickie's correction: 30 cm from Q1 entrance
	  double pHRSVBPos_X=(pHallCenter2Col+PaulColT/2) * cos(-mRHRSAngle);
	  double pHRSVBPos_Y=(pHallCenter2Col+PaulColT/2) * sin(-mRHRSAngle);

	  G4RotationMatrix *rot28=new G4RotationMatrix();
	  rot28->rotateY( 90 * deg);
	  rot28->rotateX( 90 * deg + 28 * deg);
	  G4RotationMatrix *rot28m=new G4RotationMatrix();
	  rot28m->rotateY( 90 * deg);
	  rot28m->rotateX( - 90 * deg - 28 * deg);

	  G4VSolid* collCircle1 = new G4Tubs("collCirlce1", 0,              2. * pQ1Rin,        PaulColT, 0.,               360.0 * deg);
	  G4VSolid* collCircle2 = new G4Tubs("collCirlce2", 20.5*cm-buffer2,20.5*cm,      2*PaulColT,
					     -asin(11.7/20.5), 2 * asin(11.7/20.5));
	  G4VSolid* collCircle3 = new G4Tubs("collCirlce3", 20.0*cm,      20.0*cm+buffer2,2*PaulColT, -acos(18.9/20.0), 2 * acos(18.9/20.0));
	  G4Box*    collBox1    = new G4Box ("collBox1"   , 2 * PaulColT, 11.7 * cm,   (2.33+2.9)/2.0 * cm);
	  G4Box*    collBox2    = new G4Box ("collBox2"   , 2 * PaulColT, box2h / 2, box2w / 2);
	  G4Box*    collBox3    = new G4Box ("collBox3"   ,     PaulColT, box3h / 2, box3w / 2);

	  G4SubtractionSolid* subtraction1 = new G4SubtractionSolid("subtraction1", collCircle1,  collCircle2,
								    0, G4ThreeVector(-14.5 * cm, 0., 0.));
	  //pRotX90deg, G4ThreeVector(14.5, 0., 0.));
	  G4SubtractionSolid* subtraction2 = new G4SubtractionSolid("subtraction2", subtraction1, collCircle3,
								    0, G4ThreeVector(-22.9 * cm, 0., 0.));
	  G4SubtractionSolid* subtraction3 = new G4SubtractionSolid("subtraction3", subtraction2, collBox1,
								    0, G4ThreeVector(.285 * cm, 0., 0.));
	  G4SubtractionSolid* subtraction4
	    = new G4SubtractionSolid("subtraction4", subtraction3, collBox2,
				     0, G4ThreeVector(-0.5 * box2w,
						      20 * cm * sin(acos(18.9/20.0)) + 0.5 * box2h,
						      0.));
	  G4SubtractionSolid* subtraction5
	    = new G4SubtractionSolid("subtraction5", subtraction4, collBox2,
				     0, G4ThreeVector(-0.5 * box2w,
						      -20 * cm * sin(acos(18.9/20.0)) - 0.5 * box2h,
						      0.));
	  G4UnionSolid* subtraction6
	    = new G4UnionSolid("subtraction6", subtraction5, collBox3,
			       //rot28, G4ThreeVector(- box3x - 0.5 * box3w * cos(box3deg),
			       rot28, G4ThreeVector(- box3x - 0.5 * box3h * cos(box3deg),
						    //box3y + 0.5 * box3w * sin(box3deg),
						    box3y + 0.5 * box3h * sin(box3deg),
						    0.));
	  G4UnionSolid* subtraction7
	    = new G4UnionSolid("subtraction7", subtraction6, collBox3,
			       //rot28m, G4ThreeVector(- box3x - 0.5 * box3w * cos(box3deg),
			       rot28m, G4ThreeVector(- box3x - 0.5 * box3h * cos(box3deg),
						     //- box3y - 0.5 * box3w * sin(box3deg),
						     - box3y - 0.5 * box3h * sin(box3deg),
						     0.));
	  
	  G4LogicalVolume* PaulLogical = new G4LogicalVolume(subtraction7, mMaterialManager->tungsten, "PaulLogical", 0, 0, 0);
	  G4RotationMatrix *pRotLHRScol=new G4RotationMatrix();
	  //pRotLHRScol->rotateZ(180 * deg);
	  pRotLHRScol->rotateY(-mLHRSAngle); 
	  G4RotationMatrix *pRotRHRScol=new G4RotationMatrix();
	  
	  pRotRHRScol->rotateZ(180 * deg);
	  pRotRHRScol->rotateY(mRHRSAngle); 
	  //pRotRHRScol->rotateX(90. * deg); 

	  PaulLogical->SetVisAttributes(G4VisAttributes(G4Colour::Red()));
	  
	  //new G4PVPlacement(pRotRHRScol,G4ThreeVector(-pHRSVBPos_Y,0,pHRSVBPos_X),
	  //PaulLogical,"PaulPhys",motherLogical,0,0,0);
	  //new G4PVPlacement(pRotLHRScol,G4ThreeVector(pHRSVBPos_Y,0,pHRSVBPos_X),
	  //PaulLogical,"PaulPhys",motherLogical,0,0,0);
	  //End of Paul's collimator////////////////////////////////////////////////////////////////////////   	  
	  
	}
	else if( mSnakeModel != 49 || mSnakeModel > 50)
	{
		//place VB @ Septum entrance window, 10.4cm width and 24.4cm height, 
		//The following declared as module variales already
		//double mHRSVBWidth=104*mm;
		//double mHRSVBHeight=244*mm;
		//double mHRSVBThick=0.0508*mm; 
		double mHRSVBWidth=800*mm;
		double mHRSVBHeight=800*mm;
		double mHRSVBThick=0.0508*mm; 
		//acceptance is 20 mrad
	  
		G4VSolid* septumWindowSolid = new G4Box("septumWindowBox",mHRSVBWidth/2.0,
			mHRSVBHeight/2.0,mHRSVBThick/2.0);
		G4LogicalVolume* septumWindowLogical = new G4LogicalVolume(septumWindowSolid,
		        mMaterialManager->mylar,"septumWindowLogical",0,0,0);
		SDman->AddNewDetector(septumWindowSD);
		septumWindowLogical->SetSensitiveDetector(septumWindowSD);
		septumWindowLogical->SetVisAttributes(LightYellowVisAtt); 

		G4VSolid* TargetSolid = new G4Tubs("TargetTub",0.0,50*cm,
			mHRSVBThick/2.0,0.0,360.0*deg);
		G4LogicalVolume* TargetLogical = new G4LogicalVolume(TargetSolid,
			mMaterialManager->mylar,"TargetLogical",0,0,0);
		SDman->AddNewDetector(septumWindowSD);
		TargetLogical->SetSensitiveDetector(septumWindowSD);
		TargetLogical->SetVisAttributes(LightYellowVisAtt); 

		double pTunnel2Beam_X=8.4*cm;
		//put both left and right septum entrance window, which should not less than pTunnel2Beam_X
		if(mSetupLHRS){
		  double pLSeptumWindowPos_X=(mPivot2LHRSVBFace+mHRSVBThick/2.0)*
		    sin(mLSeptumAngle)+mPivotXOffset;
		  if(mPivot2LHRSVBFace>1190*mm){
		    //need to shift it in X to make it barely touch the septum tunnel		
		    pLSeptumWindowPos_X=mHRSVBWidth/2*cos(mLSeptumAngle)+pTunnel2Beam_X;
		  }
		  double pLSeptumWindowPos_Y=mPivotYOffset;
		  double pLSeptumWindowPos_Z=(mPivot2LHRSVBFace+mHRSVBThick/2.0)*
		    cos(mLSeptumAngle)+mPivotZOffset;
		  //new G4PVPlacement(pRotLSeptum,
		  //G4ThreeVector(pLSeptumWindowPos_X,pLSeptumWindowPos_Y,pLSeptumWindowPos_Z),
		  //septumWindowLogical,"virtualBoundaryPhys_LHRS",motherLogical,0,0,0);
		}if(mSetupRHRS){
		  double pRSeptumWindowPos_X=(mPivot2RHRSVBFace+mHRSVBThick/2.)*
		    sin(mRSeptumAngle)+mPivotXOffset;			
		  if(mPivot2LHRSVBFace>1190*mm){
		    //need to shift it in X to make it barely touch the septum tunnel		
		    pRSeptumWindowPos_X=-mHRSVBWidth/2*cos(mRSeptumAngle)-pTunnel2Beam_X;
		  }
		  double pRSeptumWindowPos_Y=mPivotYOffset;
		  double pRSeptumWindowPos_Z=(mPivot2RHRSVBFace+mHRSVBThick/2.0)*
		    cos(mRSeptumAngle)+mPivotZOffset;
		  //new G4PVPlacement(pRotRSeptum,
		  //G4ThreeVector(pRSeptumWindowPos_X,pRSeptumWindowPos_Y,pRSeptumWindowPos_Z),
		  //septumWindowLogical,"virtualBoundaryPhys_RHRS",motherLogical,0,0,0);
		}
		//NICKIE ADDED ALL OF THIS
		/*
		G4VSolid* HRSVBSolid = new G4Tubs("HRSVBTub",0.0,15*cm,
			mHRSVBThick/2.0,0.0,360.0*deg);
		G4LogicalVolume* HRSVBLogical = new G4LogicalVolume(HRSVBSolid,
			mMaterialManager->mylar,"HRSVBLogical",0,0,0);
		SDman->AddNewDetector(septumWindowSD);
		HRSVBLogical->SetSensitiveDetector(septumWindowSD);
		HRSVBLogical->SetVisAttributes(LightYellowVisAtt); 
		*/
		double pHallCenter2VB=1.40*m;
		//gConfig->SetParameter("Pivot2CHRSVBFace",pHallCenter2VB-mPivotZOffset);
		gConfig->SetParameter("Pivot2CHRSVBFace",pHallCenter2VB-mPivotZOffset); 

		int    mSnakeModel;
		gConfig->GetArgument("SnakeModel",mSnakeModel);
		/*
		cout << "Snake model " << mSnakeModel << endl;
		cout << "Snake model " << mSnakeModel << endl;
		cout << "Snake model " << mSnakeModel << endl;
		cout << "Snake model " << mSnakeModel << endl;
		
		if(mSnakeModel==47){
		  double pCHRSVBPos_X=mPivotXOffset;
		  double pCHRSVBPos_Y=mPivotYOffset;
		  //no need to correct for pivot since the distance is from the hall center
		  //double pCHRSVBPos_Z=(mHRSVBThick/2.0 - 110 * cm);
		  double pCHRSVBPos_Z=( - 100 * cm ); // DANGER IN PLACEMENT
		  new G4PVPlacement(pRotNone,G4ThreeVector(pCHRSVBPos_X,pCHRSVBPos_Y,pCHRSVBPos_Z),
				    TargetLogical,"virtualBoundaryPhys_C",motherLogical,0,0,0);
		  //cout << "The angle of the HRS is: " << -mLHRSAngle << endl;
		}
		*/
		/*
		//THIS IS THE NICKIE ADDITION FOR THE PREX TUNE FUNCTIONS STARTING AT THE TARGET CENTER
		G4VSolid* HRSVBSolid = new G4Tubs("HRSVBTub",0.0,15*cm,
			mHRSVBThick/2.0,0.0,360.0*deg);
		G4LogicalVolume* HRSVBLogical = new G4LogicalVolume(HRSVBSolid,
			mMaterialManager->mylar,"HRSVBLogical",0,0,0);
		SDman->AddNewDetector(septumWindowSD);
		HRSVBLogical->SetSensitiveDetector(septumWindowSD);
		HRSVBLogical->SetVisAttributes(LightYellowVisAtt); 

		double pHallCenter2VB=1.40*m;
		gConfig->SetParameter("Pivot2LHRSVBFace",pHallCenter2VB-mPivotZOffset);
		gConfig->SetParameter("Pivot2RHRSVBFace",pHallCenter2VB-mPivotZOffset); 
		double pLHRSVBPos_X=mPivotXOffset;
		double pLHRSVBPos_Y=mPivotYOffset;
		//no need to correct for pivot since the distance is from the hall center
		double pLHRSVBPos_Z=(pHallCenter2VB-mHRSVBThick/2.0);
		new G4PVPlacement(pRotLHRS,G4ThreeVector(pLHRSVBPos_X,pLHRSVBPos_Y,pLHRSVBPos_Z),
				  HRSVBLogical,"virtualBoundaryPhys_LHRS",motherLogical,0,0,0);
		*/
	}


	return theG2PSeptumPhys;


}

g4hrsDetectorConstruction::CreateTargetChamber(G4LogicalVolume *pMotherLogVol){
	const double inch=2.54*cm;

	G4RotationMatrix *pRotScatInHall=new G4RotationMatrix();
	pRotScatInHall->rotateX(90.*deg);

	double startphi=0.*deg, deltaphi=360.*deg;
	/////////////////////////////////////////////////////////
	//scattering chamber container
	/////////////////////////////////////////////////////////
	//This is just a container to enclose the taraget chamber, 10 mm larger than the 
	//scattering chamber itself.
	//With this container, all stuff inside do not need a rotation
	
	//The scattering chamber containner is made of helium gas, 

	double pScatChamberContainerRin=mScatChamberRin-1*cm;      
	double pScatChamberContainerRout=mScatChamberRout+1*cm;
	double pScatChamberContainerL=mScatChamberL+(3.50+17.0+1.25)*inch*2+10.0*mm;
	G4VSolid* scatChamberContainerExtendedSolid = new G4Tubs("scatChamberContainerExtendedTubs",
		pScatChamberContainerRin,pScatChamberContainerRout,
		pScatChamberContainerL/2.0,0.,360.*deg);
	G4VSolid* scatChamberContainerExtraSolid = new G4Tubs("scatChamberContainerExtraTubs",
		0,pScatChamberContainerRout+1*mm,17.25*inch/2.0,0.,360.*deg);
	G4SubtractionSolid* scatChamberContainerSolid=new G4SubtractionSolid("scatChamberContainerSolid",
		scatChamberContainerExtendedSolid,scatChamberContainerExtraSolid,
		0,G4ThreeVector(0,0,-mScatChamberL/2-17.25*inch/2.0));

	G4LogicalVolume* scatChamberContainerLogical = new G4LogicalVolume(scatChamberContainerSolid,
		heliumGas,"scatChamberContainerLogical",0,0,0);
	scatChamberContainerLogical->SetVisAttributes(HallVisAtt); 

	G4VPhysicalVolume* scatChamberContainerPhys=new G4PVPlacement(pRotScatInHall,
		G4ThreeVector(mScatChamberXOffset,mScatChamberYOffset,mScatChamberZOffset),
		scatChamberContainerLogical,"scatChamberContainerPhys",pMotherLogVol,0,0);

	//////////////////////////
	// build the target chamber.
	//////////////////////////
	//Build the simplified scatter chamber,it contains 2 windows of rectangles 
	//The following already defined in the config file
	//double mScatChamberRin=17.875*inch,mScatChamberRout=18.875*inch,mScatChamberL=27.25*inch;

	G4VSolid* scatChamberWholeSolid=0;
	//If mSetupScatChamber==1, setup the body only, 
	//If mSetupScatChamber==2, setup the body plus top flange and bottom flange, this
	//will make the program slower
	if(mSetupStdScatChamber==1)
	{
	  scatChamberWholeSolid = new G4Tubs("scatChamberWholeTubs",
					     mScatChamberRin,mScatChamberRout,mScatChamberL/2.0,0.,360.*deg);
	}
	else if(mSetupStdScatChamber>=2)
	{
		startphi=0.*deg; deltaphi=360.*deg;
		const int kNPlane_SC=11;
		double rInner_SC[] = {0,0,mScatChamberRin,
			mScatChamberRin,mScatChamberRin,mScatChamberRin,
			mScatChamberRin,mScatChamberRin,mScatChamberRin,
			0,0};
		double rOuter_SC[] = {
			mScatChamberRout+1.0*inch,mScatChamberRout+1.0*inch,mScatChamberRout+1.0*inch,
			mScatChamberRout,mScatChamberRout,mScatChamberRout+1.0*inch,
			mScatChamberRout+1.0*inch,mScatChamberRout,mScatChamberRout,
			mScatChamberRout+1*inch,mScatChamberRout+1*inch
		};
		double zPlane_SC[] = {
			-mScatChamberL/2-4.50*inch,-mScatChamberL/2-3.25*inch,-mScatChamberL/2-1.0*inch,
			-mScatChamberL/2,mScatChamberL/2+0.25*inch,mScatChamberL/2+1.25*inch,
			mScatChamberL/2+3.50*inch,mScatChamberL/2+3.50*inch,mScatChamberL/2+20.5*inch,
			mScatChamberL/2+20.5*inch,mScatChamberL/2+21.75*inch
		};

		G4Polycone* SCWholeSolid = new G4Polycone("SCPolycone",startphi,deltaphi,
			kNPlane_SC,zPlane_SC,rInner_SC,rOuter_SC);

		scatChamberWholeSolid = SCWholeSolid;
	}


	//these are the subtraction part, not the scatter chamber itself
	double pSCWindowRin=mScatChamberRin-1*mm;
	double pSCWindowRout=mScatChamberRout+1*mm;
	double pSCEntranceWindowH=6.44*inch;
	double pSCDownCapH=15.0*inch;
	
	//rectangle EntranceWindow covering 80 to 100 degrees
	startphi=80*deg;deltaphi=20*deg;
	G4VSolid* SCEntranceWindowSolid = new G4Tubs("SCEntranceWindowTubs",
		pSCWindowRin,pSCWindowRout,pSCEntranceWindowH/2.0,startphi,deltaphi);
	//rectangle DownCap covering -225 to 45 degrees
	startphi=-225*deg;deltaphi=270*deg;
	G4VSolid* SCDownCapSolid = new G4Tubs("SCDownCapTubs",
		pSCWindowRin,pSCWindowRout,pSCDownCapH/2.0,startphi,deltaphi);
	// subtract the Entrance window 
	G4SubtractionSolid* SCSubtractEntranceSolid=new G4SubtractionSolid(
		"SCSubtractEntrance",scatChamberWholeSolid,SCEntranceWindowSolid);

	// subtract the Exit window 
	G4SubtractionSolid* SCSubtractEntranceNExitSolid=new G4SubtractionSolid(
		"SCSubtractEntranceNExit",SCSubtractEntranceSolid,SCDownCapSolid);

	//setup the scatter chamber
	G4LogicalVolume* scatChamberLogical = new G4LogicalVolume(
		SCSubtractEntranceNExitSolid,aluminum,"scatChamberLogical",0,0,0);
	scatChamberLogical->SetVisAttributes(WhiteVisAtt); 

	new G4PVPlacement(0,G4ThreeVector(),
		scatChamberLogical,"scatChamberPhys",scatChamberContainerLogical,0,0);

	
	/////////////////////////
	// target chamber window covers 
	/////////////////////////
	//Covers for EntranceWindow 

	//EntranceWindowCover
	double pSCEntranceWindowCoverH=pSCEntranceWindowH+0.8*inch;
	double pSCEntranceWindowCoverRin=mScatChamberRout;
	double pSCEntranceWindowCoverRout=pSCEntranceWindowCoverRin+mScatChamberEntranceWindowThick;

	startphi=78*deg;deltaphi=24*deg;
	G4VSolid* SCEntranceWindowCoverSolid = new G4Tubs("SCEntranceWindowCoverTubs",
		pSCEntranceWindowCoverRin,pSCEntranceWindowCoverRout,
		pSCEntranceWindowCoverH/2.0,startphi,deltaphi);
	G4LogicalVolume* SCEntranceWindowCoverLogical = new G4LogicalVolume(
		SCEntranceWindowCoverSolid,aluminum,"SCEntranceWindowCoverLogical",0,0,0);
	SCEntranceWindowCoverLogical->SetVisAttributes(LightYellowVisAtt); 

	new G4PVPlacement(0,G4ThreeVector(),SCEntranceWindowCoverLogical,
		"SCEntranceWindowCoverPhys",scatChamberContainerLogical,false,0);

	//DownCapCover
	double pSCDownCapCoverH=pSCDownCapH+0.8*inch;
	double pSCDownCapCoverRin=mScatChamberRout;
	double pSCDownCapCoverRout=mScatChamberRout+mScatChamberExitWindowThick;

	startphi=-227*deg;deltaphi=274*deg;
	G4VSolid* SCDownCapCoverSolid = new G4Tubs("SCDownCapCoverTubs",
		pSCDownCapCoverRin,pSCDownCapCoverRout,pSCDownCapCoverH/2.0,startphi,deltaphi);
	G4LogicalVolume* SCDownCapCoverLogical = new G4LogicalVolume(
		SCDownCapCoverSolid,aluminum,"SCDownCapCoverLogical",0,0,0);
	SCDownCapCoverLogical->SetVisAttributes(LightYellowVisAtt); 

	new G4PVPlacement(0,G4ThreeVector(),SCDownCapCoverLogical,
		"SCDownCapCoverPhys",scatChamberContainerLogical,false,0);


	return scatChamberContainerPhys;

}

g4hrsDetectorConstruction::CreateHRS(){
}





