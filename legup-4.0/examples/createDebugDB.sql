CREATE SCHEMA IF NOT EXISTS `legupDebug` DEFAULT CHARACTER SET latin1 ;
USE `legupDebug` ;

CREATE TABLE `Version` (
  `date_time` VARCHAR(100)
);

CREATE TABLE `Designs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `path` varchar(512) NOT NULL,
  `name` varchar(128) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `path_UNIQUE` (`path`)
);

CREATE TABLE `DesignProperties` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `designId` INT NOT NULL,
  `isDebugRtlEnabled` BIT(1) NOT NULL,
  `isXilinx` BIT(1) NOT NULL,
  `board` VARCHAR(45) NOT NULL,
  `memoryAddrWidth` INT NOT NULL,
  `memoryDataWidth` INT NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `designId_UNIQUE` (`designId` ASC),
  CONSTRAINT `fk_DesignOptions_1`
    FOREIGN KEY (`designId`)
    REFERENCES `legupDebug`.`Designs` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT
);

CREATE TABLE `InstrumentationProperties` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `designId` INT NOT NULL,
  `numInstanceBits` INT NOT NULL,
  `numStateBits` INT NOT NULL,
  `systemId` INT UNSIGNED NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE INDEX `designId_UNIQUE` (`designId` ASC),
  CONSTRAINT `fk_InstrumentationProperties_1`
    FOREIGN KEY (`designId`)
    REFERENCES `legupDebug`.`Designs` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT
);

CREATE TABLE `TraceBufferProperties` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `designId` INT NOT NULL,
  `controlBufWidth` INT NOT NULL,  
  `controlBufSequenceBits` INT NOT NULL,
  `controlBufDepth` INT NOT NULL,
  `memoryBufWidth` INT NOT NULL,
  `memoryBufDepth` INT NOT NULL,
  `regsBufEnabled` BIT(1) NOT NULL,
  `regsBufWidth` INT NOT NULL,
  `regsBufDepth` INT NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_TraceBufferProperties_1_idx` (`designId` ASC),
  CONSTRAINT `fk_TraceBufferProperties_1`
    FOREIGN KEY (`designId`)
    REFERENCES `legupDebug`.`Designs` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT);


CREATE TABLE `Function` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `designId` INT NOT NULL,
  `name` varchar(1024) NOT NULL,
  `inlined` BIT(1) NOT NULL,
  `hasMetadata` BIT(1) NOT NULL,
  `startLineNumber` INT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_Function_1_idx` (`designId`),
  CONSTRAINT `fk_Function_1` FOREIGN KEY (`designId`) REFERENCES `Designs` (`id`) ON UPDATE RESTRICT ON DELETE CASCADE
);

CREATE TABLE `Instance` (
    `id` int(11) NOT NULL AUTO_INCREMENT,
    `designId` int(11) NOT NULL,
    `instanceNum` int(11) NOT NULL,
    `functionId` int(11) NOT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY `uk_FunctionInstance` (`designId` , `instanceNum`),
    KEY `fk_FunctionInstance_1_idx` (`functionId`),
    KEY `fk_FunctionInstance_2_idx` (`designId`),
    CONSTRAINT `fk_FunctionInstance_1` FOREIGN KEY (`functionId`)
        REFERENCES `Function` (`id`)
        ON DELETE CASCADE
		ON UPDATE RESTRICT,
    CONSTRAINT `fk_FunctionInstance_2` FOREIGN KEY (`designId`)
        REFERENCES `Designs` (`id`)
        ON DELETE CASCADE
		ON UPDATE RESTRICT
);

CREATE TABLE `InstanceChildren` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `instanceId` INT NOT NULL,
  `childInstanceId` INT NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_new_table_1_idx` (`instanceId` ASC),
  INDEX `fk_new_table_2_idx` (`childInstanceId` ASC),
  CONSTRAINT `fk_new_table_1`
    FOREIGN KEY (`instanceId`)
    REFERENCES `legupDebug`.`Instance` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT,
  CONSTRAINT `fk_new_table_2`
    FOREIGN KEY (`childInstanceId`)
    REFERENCES `legupDebug`.`Instance` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT);


CREATE  TABLE `State` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `belongingFunctionId` INT NOT NULL ,
  `calledFunctionId` INT NULL ,
  `number` INT NOT NULL ,
  `name` VARCHAR(1024) NOT NULL,
  `storeA` BIT(1) NOT NULL,
  `storeB` BIT(1) NOT NULL,
  `traceRegsPortA` BIT(1) NOT NULL,
  `traceRegsPortB` BIT(1) NOT NULL,
  PRIMARY KEY (`id`) ,
  INDEX `fk_State_Function1_idx` (`belongingFunctionId` ASC) ,
  INDEX `fk_State_Function2_idx` (`calledFunctionId` ASC) ,
  CONSTRAINT `fk_State_Function1`
    FOREIGN KEY (`belongingFunctionId` )
    REFERENCES `legupDebug`.`Function` (`id` )
    ON DELETE CASCADE
    ON UPDATE RESTRICT,
  CONSTRAINT `fk_State_Function2`
    FOREIGN KEY (`calledFunctionId` )
    REFERENCES `legupDebug`.`Function` (`id` )
    ON DELETE CASCADE
    ON UPDATE RESTRICT);


CREATE TABLE `IRInstr` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `functionId` INT NOT NULL ,
  `numInFunction` INT NOT NULL ,
  `isDummyDebugCall` BIT(1) NOT NULL,
  `filePath` VARCHAR(1024) NULL,
  `lineNumber` INT NULL,
  `columnNumber` INT NULL,
  `dump` VARCHAR(2000) NULL ,
  `startStateId` INT NULL,
  `endStateId` INT NULL,
 PRIMARY KEY (`id`),
  INDEX `fk_IRInstr_Function1_idx` (`functionId` ASC) ,
  CONSTRAINT `fk_IRInstr_Function1`
    FOREIGN KEY (`functionId` )
    REFERENCES `legupDebug`.`Function` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT,
  CONSTRAINT `fk_IRInstr_State1`
    FOREIGN KEY (`startStateId` )
    REFERENCES `legupDebug`.`State` (`id` )
    ON DELETE CASCADE
    ON UPDATE RESTRICT,
  CONSTRAINT `fk_IRInstr_State2`
    FOREIGN KEY (`endStateId` )
    REFERENCES `legupDebug`.`State` (`id` )
    ON DELETE CASCADE
    ON UPDATE RESTRICT);

CREATE TABLE `RtlSignal` (
    `id` int NOT NULL AUTO_INCREMENT,
    `functionId` INT NOT NULL,
    `signalName` varchar(1024) NOT NULL,
    `width` INT NOT NULL,
    #`generatedStateId` int NOT NULL,    
    PRIMARY KEY (`id`),
    KEY `fk_RtlSignal_1_idx` (`functionId`),  
    CONSTRAINT `fk_RtlSignal_1` FOREIGN KEY (`functionId`)
        REFERENCES `Function` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableType` (
    `id` int NOT NULL AUTO_INCREMENT,
    `designId` int NOT NULL,
    `dw_tag` INT NOT NULL,
    `name` varchar(200) NOT NULL,
    `size` int NOT NULL,
    `alignment` int NOT NULL,
    `offset` int NOT NULL,
    `derivedTypeId` int NULL,    
    PRIMARY KEY (`id`),
    KEY `fk_VariableType_1_idx` (`designId`),  
    KEY `fk_VariableType_2_idx` (`derivedTypeId`),  
    CONSTRAINT `fk_VariableType_1` FOREIGN KEY (`designId`)
        REFERENCES `Designs` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    CONSTRAINT `fk_VariableType_2` FOREIGN KEY (`derivedTypeId`)
        REFERENCES `VariableType` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableTypeMember` (
  `id` int NOT NULL AUTO_INCREMENT,
  `ownerVariableTypeId` int NOT NULL,
  `idx` int NOT NULL,
  `variableTypeId` int NULL,
  `subrangeCount` int NULL,
  PRIMARY KEY (`id`),
  KEY `fk_VariableTypeMember_1_idx` (`ownerVariableTypeId`),  
  KEY `fk_VariableTypeMember_2_idx` (`variableTypeId`),  
  CONSTRAINT `fk_VariableTypeMember_1` FOREIGN KEY (`ownerVariableTypeId`)
        REFERENCES `VariableType` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `fk_VariableTypeMember_2` FOREIGN KEY (`variableTypeId`)
        REFERENCES `VariableType` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `Variable` (
    `id` int NOT NULL AUTO_INCREMENT,
    `designId` int NOT NULL,
    `name` varchar(1024) NOT NULL,
    `isGlobal` bit(1) NOT NULL,
    `origFunctionId` int DEFAULT NULL,
    `functionId` int DEFAULT NULL,
    `typeId` int NOT NULL,
    `filePath` VARCHAR(1024) NULL,
    `lineNumber` INT NULL,
    `inlinedPath` VARCHAR(4096) NULL,
    PRIMARY KEY (`id`),
    KEY `fk_Variable_Function1_idx` (`functionId`),
    KEY `fk_Variable_VariableType1_idx` (`typeId`),
    KEY `fk_Variable_1_idx` (`designId`),
    CONSTRAINT `fk_Variable_1` FOREIGN KEY (`designId`)
        REFERENCES `Designs` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    CONSTRAINT `fk_Variable_Function1` FOREIGN KEY (`functionId`)
        REFERENCES `Function` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    CONSTRAINT `fk_Variable_Function2` FOREIGN KEY (`origFunctionId`)
        REFERENCES `Function` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    CONSTRAINT `fk_Variable_VariableType1` FOREIGN KEY (`typeId`)
        REFERENCES `VariableType` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);


CREATE TABLE `VariableSource` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `variableId` INT NOT NULL,
  `IRInstrId` INT NULL,
  `valSrcSupported` BIT(1) NOT NULL,
  PRIMARY KEY (`id`),
  INDEX `fk_VariableSource_1_idx` (`variableId` ASC),
  INDEX `fk_VariableSource_2_idx` (`IRInstrId` ASC),
  CONSTRAINT `fk_VariableSource_1`
    FOREIGN KEY (`variableId`)
    REFERENCES `legupDebug`.`Variable` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT,
  CONSTRAINT `fk_VariableSource_2`
    FOREIGN KEY (`IRInstrId`)
    REFERENCES `legupDebug`.`IRInstr` (`id`)
    ON DELETE CASCADE
    ON UPDATE RESTRICT);

CREATE TABLE `RAM` (
    `id` int NOT NULL AUTO_INCREMENT,
    `designId` int NOT NULL,
    `tag` varchar(200) NOT NULL,
    `tagNum` int NOT NULL,
    `tagAddressName` varchar(200) NOT NULL,
    `addressWidth` int NOT NULL,
    `mifFileName` varchar(200) NOT NULL,
    `dataWidth` int NOT NULL,
    `numElements` int NOT NULL,
    PRIMARY KEY (`id`),
    KEY `fk_RAM_1_idx` (`designId`),
    CONSTRAINT `fk_RAM_1` FOREIGN KEY (`designId`)
        REFERENCES `Designs` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableSourceRAM` (
    `id` int NOT NULL AUTO_INCREMENT,
    `VariableSourceId` int NOT NULL,
    `ramId` int NOT NULL,
    PRIMARY KEY (`id`),
    KEY `fk_VariableSourceRAM_1_idx` (`VariableSourceId`),
    KEY `fk_VariableSourceRAM_2_idx` (`ramId`),
    CONSTRAINT `fk_VariableSourceRAM_1` FOREIGN KEY (`VariableSourceId`)
        REFERENCES `VariableSource` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    CONSTRAINT `fk_VariableSourceRAM_2` FOREIGN KEY (`ramId`)
        REFERENCES `RAM` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableSourceConstantInt` (
    `id` int NOT NULL AUTO_INCREMENT,
    `VariableSourceId` int NOT NULL,
    `constantInt` BIGINT NULL,    
    PRIMARY KEY (`id`),
    KEY `fk_VariableSourceConstantInt_1_idx` (`VariableSourceId`),
    CONSTRAINT `fk_VariableSourceConstantInt_1` FOREIGN KEY (`VariableSourceId`)
        REFERENCES `VariableSource` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableSourceUndefined` (
    `id` int NOT NULL AUTO_INCREMENT,
    `VariableSourceId` int NOT NULL,    
    PRIMARY KEY (`id`),
    KEY `fk_VariableSourceUndefined_1_idx` (`VariableSourceId`),
    CONSTRAINT `fk_VariableSourceUndefined_1` FOREIGN KEY (`VariableSourceId`)
        REFERENCES `VariableSource` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableSourcePointer` (
    `id` int NOT NULL AUTO_INCREMENT,
    `VariableSourceId` int NOT NULL,
    `ramId` int NOT NULL,
    `offset` int NOT NULL,
    PRIMARY KEY (`id`),
    KEY `fk_VariableSourcePointer_1_idx` (`VariableSourceId`),
    KEY `fk_VariableSourcePointer_2_idx` (`ramId`),
    CONSTRAINT `fk_VariableSourcePointer_1` FOREIGN KEY (`VariableSourceId`)
        REFERENCES `VariableSource` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    CONSTRAINT `fk_VariableSourcePointer_2` FOREIGN KEY (`ramId`)
        REFERENCES `RAM` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);

CREATE TABLE `VariableSourceSignal` (
    `id` int NOT NULL AUTO_INCREMENT,
    `VariableSourceId` int NOT NULL,
    `rtlSignalId` int NOT NULL,    
    PRIMARY KEY (`id`),
    KEY `fk_VariableSourceSignal_1_idx` (`VariableSourceId`),    
    CONSTRAINT `fk_VariableSourceSignal_1` FOREIGN KEY (`VariableSourceId`)
        REFERENCES `VariableSource` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
    KEY `fk_VariableSourceSignal_2_idx` (`rtlSignalId`),    
    CONSTRAINT `fk_VariableSourceSignal_2` FOREIGN KEY (`rtlSignalId`)
        REFERENCES `RtlSignal` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT 
);

CREATE TABLE `RtlSignalTraceSchedule` (
    `id` int NOT NULL AUTO_INCREMENT,
    `rtlSignalId` int NOT NULL,
    `delayedCycles` int NOT NULL,
    `recordInStateId` int NULL,
    `hiBit` int NOT NULL,
    `loBit` int NOT NULL,
    PRIMARY KEY (`id`),
    KEY `fk_SignalTraceState_1_idx` (`rtlSignalId`),
    KEY `fk_SignalTraceState_2_idx` (`recordInStateId`),
    CONSTRAINT `fk_SignalTraceState_1` FOREIGN KEY (`rtlSignalId`)
        REFERENCES `RtlSignal` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT,
  CONSTRAINT `fk_SignalTraceState_2` FOREIGN KEY (`recordInStateId`)
        REFERENCES `State` (`id`)
        ON DELETE CASCADE ON UPDATE RESTRICT
);
