/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/



#include "private.h"
#include "workbench.h"
#include "terrainpanel.h"
#include "filedialog.h"

typedef MapTool editorTool;

TerrainPanel::TerrainPanel(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);

	m_actionGroup = new QActionGroup(this);
	m_actionGroup->setExclusive(true);
	m_actionGroup->addAction(ui.actionHeightEdit);
	m_actionGroup->addAction(ui.actionTerrainPaint);
	m_actionGroup->addAction(ui.actionMakeHole);
	m_actionGroup->addAction(ui.actionActive_Area);
	m_actionGroup->addAction(ui.actionEnv);
	m_actionGroup->addAction(ui.actionGrassTreeVegetation);

	ui.heighEdit->setDefaultAction(ui.actionHeightEdit);
	ui.terrainPaint->setDefaultAction(ui.actionTerrainPaint);
	ui.activeArea->setDefaultAction(ui.actionActive_Area);
	ui.environment->setDefaultAction(ui.actionEnv);
	ui.vegetation->setDefaultAction(ui.actionGrassTreeVegetation);
	ui.riverRoad->setDefaultAction(ui.actionRiverRoad);

#if 0
	mModifyTerrainPanel = new ModifyTerrainPanel();
	mTerrainPaintPanel = new TerrainPaintPanel();
	ui.rollupWidget->m_addPage(mModifyTerrainPanel, tr("Modify MapTerrain"));
	ui.rollupWidget->m_addPage(mTerrainPaintPanel, tr("Paint MapTerrain"));
#else
	ui.rollupWidget->initialize();
#endif
	ui.toolButton_1->setDefaultAction(ui.actionRaise );
	ui.toolButton_2->setDefaultAction(ui.actionLower );
	ui.toolButton_3->setDefaultAction(ui.actionLevel );
	ui.toolButton_4->setDefaultAction(ui.actionSmooth );
	ui.toolButton_5->setDefaultAction(ui.actionGrab );
	ui.toolButton_6->setDefaultAction(ui.actionPush );
	ui.paint->setDefaultAction(ui.actionPaint);
	ui.erase->setDefaultAction(ui.actionErase);
	ui.paintActive->setDefaultAction(ui.actionPaintActive);
	ui.eraseActBtn->setDefaultAction(ui.actionEraseActive);
	// timlly add
	ui.grassPaint->setDefaultAction(ui.actionGrassPaint);
	ui.grassErase->setDefaultAction(ui.actionGrassErase);
	ui.treePaint->setDefaultAction(ui.actionTreePaint);
	ui.treeErase->setDefaultAction(ui.actionTreeErase);
	ui.road->setDefaultAction(ui.actionRoad);
	ui.river->setDefaultAction(ui.actionRiver);

	g_workbench->addEditorTool(editorTool::TerrainRaise, ui.actionRaise );
	g_workbench->addEditorTool(editorTool::TerrainLower, ui.actionLower );
	g_workbench->addEditorTool(editorTool::TerrainLevel, ui.actionLevel );
	g_workbench->addEditorTool(editorTool::TerrainSmooth, ui.actionSmooth );
	g_workbench->addEditorTool(editorTool::TerrainGrab, ui.actionGrab );
	g_workbench->addEditorTool(editorTool::TerrainPush, ui.actionPush );
	g_workbench->addEditorTool(editorTool::TerrainPaint, ui.actionPaint );
	g_workbench->addEditorTool(editorTool::TerrainErase, ui.actionErase );
	g_workbench->addEditorTool(editorTool::TerrainPaintActive, ui.actionPaintActive );
	g_workbench->addEditorTool(editorTool::TerrainEraseActive, ui.actionEraseActive );
	// timlly add
	g_workbench->addEditorTool(editorTool::GrassPaint, ui.actionGrassPaint);
	g_workbench->addEditorTool(editorTool::GrassErase, ui.actionGrassErase);
	g_workbench->addEditorTool(editorTool::TreePaint, ui.actionTreePaint);
	g_workbench->addEditorTool(editorTool::TreeErase, ui.actionTreeErase);

	g_workbench->addEditorTool(editorTool::Road, ui.actionRoad);
	g_workbench->addEditorTool(editorTool::River, ui.actionRiver);

	setParams();

	ui.layerList->header()->hide();

//	ui.heighEdit->setChecked(true);
//	on_actionHeightEdit_triggered();
	ui.rollupWidget->setAllPagesVisible(false);

	// set envprops
	MapEnvDef *def = g_mapContext->getGameWorld()->getEnvironment();
	ui.envProps->initFromObject(def);

	g_mapContext->addObserver(this);

	connect(ui.grassColor, SIGNAL(colorEdited(const QColor&)), this, SLOT(on_grassColor_colorChanged(const QColor&)));
}

TerrainPanel::~TerrainPanel()
{
	g_mapContext->removeObserver(this);

}

void TerrainPanel::beNotified(IObservable *subject, int arg) {
	MapContext *context = dynamic_cast<MapContext*>(subject);
	if (context != g_mapContext)
		return;

	if (arg & Context::EnvironmentChanged) {
		MapEnvDef *def = g_mapContext->getGameWorld()->getEnvironment();
		ui.envProps->initFromObject(def);
	}

	if (!(arg & Context::TerrainMaterialEdited))
		return;

	ui.layerList->clear();
#if 0
	String blocksName;
	int numBlocks = 0//gEditorActiveAreaMgr->getNumBlocks();
	ui.tableWidget->setRowCount(numBlocks);
	for (int i = 0;i<numBlocks;i++){
		blocksName = gEditorActiveAreaMgr->getBlocksName(i);
		AreaBlocks *ab = gEditorActiveAreaMgr->getBlocks(blocksName);
		if (ab){
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setText(u2q(ab->name()));
			item->setData(Qt::UserRole,u2q(ab->realName()));
			ui.tableWidget->setItem(i, 0, item);
			ui.tableWidget->setRowHeight(i,16);
			//QTableWidgetItem *item = ui.tableWidget->item(0,0);
			//item->setText(u2q(ab->name()));
			//item->setData(Qt::UserRole,u2q(ab->realName()));
		}
	}
#endif

	MapTerrain *terrain = context->getTerrain();

	if (!terrain)
		return;

	MapMaterialDef *matdef = terrain->getMaterialDef();

	if (!matdef)
		return;

	for (int i = 0; i < matdef->getNumLayers(); i++) {
		MapLayerDef *l = matdef->getLayerDef(i);
		QTreeWidgetItem *item = new QTreeWidgetItem(ui.layerList);
		item->setText(0, u2q(matdef->getLayerDef(i)->name));
		item->setData(0, Qt::UserRole, l->id);
	}
}

// get param from spin, set to system and slider
void TerrainPanel::setParams() {
	g_mapContext->getMapState()->terrainBrushSize = ui.brushRadiusSpin->value();
	g_mapContext->getMapState()->terrainBrushSoftness = ui.brushSoftnessSpin->value() * 0.01f;
	g_mapContext->getMapState()->terrainBrushStrength = ui.brushStrengthSpin->value() * 0.01f;

	ui.RadiusSlider->setValue(ui.brushRadiusSpin->value());
	ui.SoftnessSlider->setValue(ui.brushSoftnessSpin->value());
	ui.StrengthSlider->setValue(ui.brushStrengthSpin->value());

	QTreeWidgetItem *item = ui.layerList->currentItem();
	if (!item)
		g_mapContext->getMapState()->terrainCurLayerId = -1;
	else
		g_mapContext->getMapState()->terrainCurLayerId = item->data(0, Qt::UserRole).toInt();
}

// get param from slider, set to system and spin
void TerrainPanel::setParamsFramSlider() {
	g_mapContext->getMapState()->terrainBrushSize = ui.RadiusSlider->value();
	g_mapContext->getMapState()->terrainBrushSoftness = ui.SoftnessSlider->value() * 0.01f;
	g_mapContext->getMapState()->terrainBrushStrength = ui.StrengthSlider->value() * 0.01f;

	ui.brushRadiusSpin->setValue(ui.RadiusSlider->value());
	ui.brushSoftnessSpin->setValue(ui.SoftnessSlider->value());
	ui.brushStrengthSpin->setValue(ui.StrengthSlider->value());

	QTreeWidgetItem *item = ui.layerList->currentItem();
	if (!item)
		g_mapContext->getMapState()->terrainCurLayerId = -1;
	else
		g_mapContext->getMapState()->terrainCurLayerId = item->data(0, Qt::UserRole).toInt();
}

void TerrainPanel::on_brushRadiusSpin_valueChanged(int)
{
	setParams();
}

void TerrainPanel::on_brushSoftnessSpin_valueChanged(int)
{
	setParams();
}

void TerrainPanel::on_brushStrengthSpin_valueChanged(int)
{
	setParams();
}

void TerrainPanel::on_layerList_currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)
{
	setParams();
}


void TerrainPanel::on_actionHeightEdit_triggered()
{
	ui.rollupWidget->setAllPagesVisible(false);
	ui.rollupWidget->setPageVisible(0, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_actionTerrainPaint_triggered()
{
	ui.rollupWidget->setAllPagesVisible(false);
	ui.rollupWidget->setPageVisible(1, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_actionVegetation_triggered()
{
	/*ui.rollupWidget->setPageVisible(0, false);
	ui.rollupWidget->setPageVisible(1, false);
	ui.rollupWidget->setPageVisible(2, false);
	ui.rollupWidget->setPageVisible(3, false);
	ui.rollupWidget->setPageVisible(4, false);
	ui.rollupWidget->setPageVisible(5, false);
	ui.rollupWidget->setPageVisible(6, false);
	ui.rollupWidget->setPageVisible(7, true);*/

	ui.rollupWidget->setAllPagesVisible(false);
	//ui.rollupWidget->setPageVisible(0, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_actionEnv_triggered()
{
	ui.rollupWidget->setAllPagesVisible(false);
	ui.rollupWidget->setPageVisible(2, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_applyEnv_clicked()
{
	MapEnvDef *def = g_mapContext->getGameWorld()->getEnvironment();
	ui.envProps->applyToObject(def);

	g_mapContext->getGameWorld()->updateEnvdef();
}

void TerrainPanel::on_RadiusSlider_valueChanged(int)
{
	setParamsFramSlider();
}

void TerrainPanel::on_SoftnessSlider_valueChanged(int)
{
	setParamsFramSlider();
}

void TerrainPanel::on_StrengthSlider_valueChanged(int)
{
	setParamsFramSlider();
}

void TerrainPanel::on_actionActive_Area_triggered()
{
	ui.rollupWidget->setAllPagesVisible(false);
	ui.rollupWidget->setPageVisible(3, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_actionGrassTreeVegetation_triggered()
{
	ui.rollupWidget->setAllPagesVisible(false);
	ui.rollupWidget->setPageVisible(4, true);
	ui.rollupWidget->setPageVisible(5, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_actionRiverRoad_triggered()
{
	ui.rollupWidget->setAllPagesVisible(false);
	ui.rollupWidget->setPageVisible(6, true);
	ui.rollupWidget->setPageVisible(7, true);
}

void TerrainPanel::on_actionRoad_triggered()
{
	
}

void TerrainPanel::on_actionRiver_triggered()
{
	
}

void TerrainPanel::on_toolButton_1_toggled(bool)
{

}

void TerrainPanel::on_toolButton_1_clicked(bool)
{

}

void TerrainPanel::on_heighEdit_clicked()
{

}


void TerrainPanel::on_checkBox_6_clicked()
{
	setParams();
}

void TerrainPanel::on_checkBox_5_clicked()
{
	setParams();
}

void TerrainPanel::on_toolButton_clicked()
{
#if 0
	gEditorActiveAreaMgr->blocksFactory();
	String blocksName;
	int numBlocks = gEditorActiveAreaMgr->getNumBlocks();
	ui.tableWidget->setRowCount(numBlocks);
	for (int i = 0;i<numBlocks;i++){
		blocksName = gEditorActiveAreaMgr->getBlocksName(i);
		AreaBlocks *ab = gEditorActiveAreaMgr->getBlocks(blocksName);
		if (ab){
			QTableWidgetItem *item = new QTableWidgetItem();
			item->setText(u2q(ab->name()));
			item->setData(Qt::UserRole,u2q(ab->realName()));
			ui.tableWidget->setItem(i, 0, item);
			ui.tableWidget->setRowHeight(i,16);
		}
	}
#endif
}

void TerrainPanel::on_toolButton_7_clicked()
{
#if 0
	QTableWidgetItem *item = ui.tableWidget->currentItem();
	if (item){
		String delName = q2u(item->text());
		gEditorActiveAreaMgr->delBlocks(delName);
		int numBlocks = gEditorActiveAreaMgr->getNumBlocks();
		ui.tableWidget->setRowCount(numBlocks);
		for (int i = 0;i<numBlocks;i++){
			String blocksName = gEditorActiveAreaMgr->getBlocksName(i);
			AreaBlocks *ab = gEditorActiveAreaMgr->getBlocks(blocksName);
			if (ab){
				QTableWidgetItem *item = new QTableWidgetItem();
				item->setText(u2q(ab->name()));
				item->setData(Qt::UserRole,u2q(ab->realName()));
				ui.tableWidget->setItem(i, 0, item);
				ui.tableWidget->setRowHeight(i,16);
			}
		}
	}
#endif
}

void TerrainPanel::on_tableWidget_cellChanged(int,int)
{
	//setParams();
}

void TerrainPanel::on_tableWidget_currentCellChanged(int,int,int,int)
{
	setParams();
}

void TerrainPanel::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
//	gEditorActiveAreaMgr->setBlocksName(q2u(item->data(Qt::UserRole).toString()),q2u(item->text()));
}

// ----------------- grasses info -----------------

void TerrainPanel::on_numGrassPerLevel_editingFinished()
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassTypeAdd_released()
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassTypeDelete_released()
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassTypes_itemSelectionChanged()
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassIncidence_valueChanged(double value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassType_currentIndexChanged(int value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassWidth_valueChanged(double value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassHeight_valueChanged(double value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassMinScale_valueChanged(double value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassMaxScale_valueChanged(double value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_textureBtn_released()
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassName_textEdited(QString text)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_grassColor_colorChanged(const QColor &color)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}

void TerrainPanel::on_modelBtn_released()
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}


void TerrainPanel::on_grassTransferZ_valueChanged(double value)
{
	MapTerrain *terrain = g_mapContext->getTerrain();
	if (!terrain)
	{
		return;
	}
}
