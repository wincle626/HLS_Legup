SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

CREATE SCHEMA IF NOT EXISTS `inspect_db` DEFAULT CHARACTER SET latin1 ;
USE `inspect_db` ;

-- -----------------------------------------------------
-- Table `inspect_db`.`Function`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`Function` (
  `id` INT(11) NOT NULL AUTO_INCREMENT ,
  `name` VARCHAR(100) NULL DEFAULT NULL ,
  `startLineNumber` INT(11) NULL ,
  PRIMARY KEY (`id`) )
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1
COLLATE = latin1_swedish_ci;


-- -----------------------------------------------------
-- Table `inspect_db`.`HLStatement`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`HLStatement` (
  `id` INT(11) NOT NULL AUTO_INCREMENT ,
  `line_number` INT(11) NULL DEFAULT NULL ,
  `column_number` INT(11) NULL ,
  `file_name` VARCHAR(100) NULL DEFAULT NULL ,
  PRIMARY KEY (`id`) )
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `inspect_db`.`IRInstr`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`IRInstr` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `number_in_function` INT(11) NOT NULL ,
  `function_id` INT(11) NOT NULL ,
  `HLStatement_id` INT(11) NULL ,
  `dump` VARCHAR(2000) NULL ,
  INDEX `fk_IRInstr_HLInstr_idx` (`HLStatement_id` ASC) ,
  INDEX `fk_IRInstr_Function1_idx` (`function_id` ASC) ,
  PRIMARY KEY (`id`) ,
  CONSTRAINT `fk_IRInstr_HLInstr`
    FOREIGN KEY (`HLStatement_id` )
    REFERENCES `inspect_db`.`HLStatement` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_IRInstr_Function1`
    FOREIGN KEY (`function_id` )
    REFERENCES `inspect_db`.`Function` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `inspect_db`.`State`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`State` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `belongingFunctionId` INT(11) NOT NULL ,
  `calledFunctionId` INT(11) NULL ,
  `number` INT NOT NULL ,
  `design_name` VARCHAR(200) NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_State_Function1_idx` (`belongingFunctionId` ASC) ,
  INDEX `fk_State_Function2_idx` (`calledFunctionId` ASC) ,
  CONSTRAINT `fk_State_Function1`
    FOREIGN KEY (`belongingFunctionId` )
    REFERENCES `inspect_db`.`Function` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_State_Function2`
    FOREIGN KEY (`calledFunctionId` )
    REFERENCES `inspect_db`.`Function` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1
COLLATE = latin1_swedish_ci;


-- -----------------------------------------------------
-- Table `inspect_db`.`IRState`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`IRState` (
  `IRInstr_id` INT NOT NULL ,
  `StartStateId` INT NOT NULL ,
  `EndStateId` INT NOT NULL ,
  INDEX `fk_IRState_State1_idx` (`StartStateId` ASC) ,
  INDEX `fk_IRState_State2_idx` (`EndStateId` ASC) ,
  PRIMARY KEY (`IRInstr_id`, `StartStateId`, `EndStateId`) ,
  INDEX `fk_IRState_IRInstr1_idx` (`IRInstr_id` ASC) ,
  CONSTRAINT `fk_IRState_State1`
    FOREIGN KEY (`StartStateId` )
    REFERENCES `inspect_db`.`State` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_IRState_State2`
    FOREIGN KEY (`EndStateId` )
    REFERENCES `inspect_db`.`State` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_IRState_IRInstr1`
    FOREIGN KEY (`IRInstr_id` )
    REFERENCES `inspect_db`.`IRInstr` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1
COLLATE = latin1_swedish_ci;


-- -----------------------------------------------------
-- Table `inspect_db`.`HardwareInfo`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`HardwareInfo` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `IRid` INT NOT NULL ,
  `info` TEXT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_HardwareInfo_IRInstr1_idx` (`IRid` ASC) ,
  CONSTRAINT `fk_HardwareInfo_IRInstr1`
    FOREIGN KEY (`IRid` )
    REFERENCES `inspect_db`.`IRInstr` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`VariableType`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`VariableType` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `typeId` INT NOT NULL ,
  `numElements` INT NOT NULL ,
  `byteSize` INT NOT NULL ,
  PRIMARY KEY (`id`) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`Variable`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`Variable` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `name` VARCHAR(200) NOT NULL ,
  `Function_id` INT(11) NULL ,
  `tag` VARCHAR(200) NOT NULL ,
  `tagNum` INT NOT NULL ,
  `tagAddressName` VARCHAR(200) NOT NULL ,
  `addressWidth` INT(11) NOT NULL ,
  `mifFileName` VARCHAR(200) NOT NULL ,
  `dataWidth` INT(11) NOT NULL ,
  `numElements` INT(11) NOT NULL ,
  `isStruct` TINYINT(1) NOT NULL ,
  `IRInstr_id` INT NULL ,
  `type` VARCHAR(45) NOT NULL ,
  `debugTypeId` INT NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_Variable_Function1_idx` (`Function_id` ASC) ,
  INDEX `fk_Variable_IRInstr1_idx` (`IRInstr_id` ASC) ,
  INDEX `fk_Variable_VariableType1_idx` (`debugTypeId` ASC) ,
  CONSTRAINT `fk_Variable_Function1`
    FOREIGN KEY (`Function_id` )
    REFERENCES `inspect_db`.`Function` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_Variable_IRInstr1`
    FOREIGN KEY (`IRInstr_id` )
    REFERENCES `inspect_db`.`IRInstr` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_Variable_VariableType1`
    FOREIGN KEY (`debugTypeId` )
    REFERENCES `inspect_db`.`VariableType` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`HWSignal`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`HWSignal` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `name` VARCHAR(100) NULL ,
  `width` INT(11) NULL ,
  `function_id` INT(11) NOT NULL ,
  `isConst` TINYINT(1) NOT NULL ,
  `Variable_id` INT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_Signal_Function1_idx` (`function_id` ASC) ,
  INDEX `fk_HWSignal_Variable1_idx` (`Variable_id` ASC) ,
  CONSTRAINT `fk_Signal_Function1`
    FOREIGN KEY (`function_id` )
    REFERENCES `inspect_db`.`Function` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_HWSignal_Variable1`
    FOREIGN KEY (`Variable_id` )
    REFERENCES `inspect_db`.`Variable` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`InstructionSignal`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`InstructionSignal` (
  `HWSignal_id` INT NOT NULL ,
  `IRInstr_id` INT NOT NULL ,
  INDEX `fk_InstructionSignal_HWSignal1_idx` (`HWSignal_id` ASC) ,
  INDEX `fk_InstructionSignal_IRInstr1_idx` (`IRInstr_id` ASC) ,
  CONSTRAINT `fk_InstructionSignal_HWSignal1`
    FOREIGN KEY (`HWSignal_id` )
    REFERENCES `inspect_db`.`HWSignal` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_InstructionSignal_IRInstr1`
    FOREIGN KEY (`IRInstr_id` )
    REFERENCES `inspect_db`.`IRInstr` (`id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`StateStoreInfo`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`StateStoreInfo` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `State_id` INT NOT NULL ,
  `HWSignal_id` INT NOT NULL ,
  `port` CHAR(1) NOT NULL ,
  `offset` INT NULL ,
  `IRInstr_id` INT NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_StateStoreInfo_State1_idx` (`State_id` ASC) ,
  INDEX `fk_StateStoreInfo_HWSignal1_idx` (`HWSignal_id` ASC) ,
  INDEX `fk_StateStoreInfo_IRInstr1_idx` (`IRInstr_id` ASC) ,
  CONSTRAINT `fk_StateStoreInfo_State1`
    FOREIGN KEY (`State_id` )
    REFERENCES `inspect_db`.`State` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_StateStoreInfo_HWSignal1`
    FOREIGN KEY (`HWSignal_id` )
    REFERENCES `inspect_db`.`HWSignal` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_StateStoreInfo_IRInstr1`
    FOREIGN KEY (`IRInstr_id` )
    REFERENCES `inspect_db`.`IRInstr` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`VariableData`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`VariableData` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `type` INT(11) NOT NULL ,
  `order_num` INT(11) NOT NULL ,
  `value` VARCHAR(50) NOT NULL ,
  `Variable_id` INT NOT NULL ,
  PRIMARY KEY (`id`) ,
  INDEX `fk_VariableData_Variable1_idx` (`Variable_id` ASC) ,
  CONSTRAINT `fk_VariableData_Variable1`
    FOREIGN KEY (`Variable_id` )
    REFERENCES `inspect_db`.`Variable` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `inspect_db`.`TypeElement`
-- -----------------------------------------------------
CREATE  TABLE IF NOT EXISTS `inspect_db`.`TypeElement` (
  `parentTypeId` INT NOT NULL ,
  `ElementTypeId` INT NOT NULL ,
  INDEX `fk_TypeElement_VariableType1_idx` (`parentTypeId` ASC) ,
  INDEX `fk_TypeElement_VariableType2_idx` (`ElementTypeId` ASC) ,
  PRIMARY KEY (`parentTypeId`, `ElementTypeId`) ,
  CONSTRAINT `fk_TypeElement_VariableType1`
    FOREIGN KEY (`parentTypeId` )
    REFERENCES `inspect_db`.`VariableType` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `fk_TypeElement_VariableType2`
    FOREIGN KEY (`ElementTypeId` )
    REFERENCES `inspect_db`.`VariableType` (`id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB;

USE `inspect_db` ;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
