/* 
 * File:   STPCreator.cpp
 * Author: nazanin
 * This class creates a STP file for SignalTap II Logic Analyzer
 * Created on September 11, 2013, 11:46 AM
 */

#include <algorithm>
#include <utility>

#include "STPCreator.h"
#include "Globals.h"

STPCreator::STPCreator(std::vector<HWSignal*>& onChipWatchSignals, std::map<std::string, int>& selectedExtraSignals, std::map<std::string, int>& byDefaultAddedSignals, OnChipDebugMode signalSelectionMode) {
    this->onChipWatchSignals = onChipWatchSignals;
    this->selectedExtraSignals = selectedExtraSignals;
    this->byDefaultAddedSignals = byDefaultAddedSignals;
    this->signalSelectionMode = signalSelectionMode;
}

STPCreator::~STPCreator() {
}

xml_node<>* STPCreator::createXMLNode(xml_document<> *doc, node_type type, std::string nodeName, std::string nodeValue) {                
    nodeName = nodeName + "\0";
    nodeValue = nodeValue + "\0";
    char* nodeChr = doc->allocate_string(nodeName.c_str());
    char* valChr = doc->allocate_string(nodeValue.c_str());
    xml_node<> *node = doc->allocate_node(type, nodeChr, valChr);
    return node;
}

xml_attribute<>* STPCreator::createXMLAttribute(xml_document<> *doc, std::string key, std::string value) {    
    key = key + "\0";
    value = value + "\0";
    char* keyChr = doc->allocate_string(key.c_str());
    char* valChr = doc->allocate_string(value.c_str());
    xml_attribute<> *attribute = doc->allocate_attribute(keyChr, valChr);
    return attribute;
}

void STPCreator::addSignalsListToSTP(std::map<std::string, int>& signalsList, std::vector<std::string>& allSignalsList) {
    std::map<std::string, int>::iterator sit;
    for (sit = signalsList.begin(); sit != signalsList.end(); sit++)
    {
        std::string sig = (*sit).first;
        
        /*if (std::find(allSignalsList.begin(), allSignalsList.end(), sig) != allSignalsList.end())
            continue;//skipping the signals which are already considered (if any).*/
        
        allSignalsList.push_back(sig);
        for (int l = 0 ; l < nodeFileLines.size(); l++) {
            std::string line = nodeFileLines[l];
            if (line.find(sig) != std::string::npos) {
                std::vector<std::string> tokens = split(line, ';');
                if (tokens[1].find("bus") != std::string::npos)
                    continue;
                if (tokens[0].find("~") != std::string::npos)
                    continue;
                FPGANode* node = new FPGANode();
                std::vector<std::string> s = split(tokens[0], '[');
                
                node->fullName = s[0];
                node->type = tokens[1];
                node->creator = tokens[2];
                node->width = (*sit).second;
                signalsToNodesList[sig].push_back(node);
                
                //break;
            }
        }
    }
}

void STPCreator::generateSTPForTimingSim(int triggerValue) {
    
    nodeFileLines.clear();
    //signalsToNodes.clear();
    signalsToNodesList.clear();
    
    std::ifstream in;
    in.open((workDir + nodeNamesFilename).c_str());
    
    while(in.good()) {
        std::string line;
        getline(in, line);
        nodeFileLines.push_back(line);
    }
    in.close();
    
    std::vector<std::string> allSignalsList;
    std::vector<std::string> allFullNameSignalsList;
        
    //adding all cur_state signals...
    std::vector<HWSignal*> curStateSignals = DA->getAllCurStateSignals();
    for (int i = 0 ; i < curStateSignals.size(); i++)
        onChipWatchSignals.insert(onChipWatchSignals.begin(), curStateSignals[i]);
    
    //adding all finish signals...
    std::vector<HWSignal*> finishSignals = DA->getAllFinishSignals();
    for (int i = 0 ; i < finishSignals.size(); i++)
        onChipWatchSignals.insert(onChipWatchSignals.begin(), finishSignals[i]);
    
    std::map<std::string, int> onChipWatchSignalsMap;
    for (int j = 0 ; j < onChipWatchSignals.size(); j++) {
        HWSignal* sig = onChipWatchSignals[j];
        std::string partialNodeName = sig->getFPGAPartialNodeName();
        int width = sig->width;
        onChipWatchSignalsMap[partialNodeName] = width;
    }
    
    addSignalsListToSTP(onChipWatchSignalsMap, allSignalsList);
     
    
    std::ifstream deviceInfoStream;
    deviceInfoStream.open((workDir + deviceInfoFileName).c_str());
    
    std::string line;
    getline(deviceInfoStream, line);
    std::string jtag_chain = split(line, ';')[1];
    
    getline(deviceInfoStream, line);
    std::string jtag_device = split(line, ';')[1];    
    deviceInfoStream.close();
    
    //std::string jtag_chain = "USB-Blaster [2-1.2]";
    //std::string jtag_device = "@1: EP3C120/EP4CE115 (0x020F70DD)";
    std::string sof_file = "";
    std::string instance_name = "auto_signaltap_0";
    std::string signal_set_name = "signal_set_1";
    std::string trigger_name = "trigger_1";
    std::string ram_type = "M4K";
    std::string sample_depth = "128";
    
    std::ofstream out;
    out.open((workDir + stpFilename).c_str());    
    
    
    xml_document<> doc;    
            
    
    
    xml_node<> *root = createXMLNode(&doc, node_element, "session", "");        
    root->append_attribute(createXMLAttribute(&doc, "jtag_chain", jtag_chain));
    root->append_attribute(createXMLAttribute(&doc, "jtag_device", jtag_device));
    root->append_attribute(createXMLAttribute(&doc, "sof_file", sof_file));
    doc.append_node(root);
        
    
    xml_node<> *display_tree = createXMLNode(&doc, node_element, "display_tree", "");
    display_tree->append_attribute(createXMLAttribute(&doc, "gui_logging_enabled", "0"));
    root->append_node(display_tree);
    
    xml_node<> *display_branch = createXMLNode(&doc, node_element, "display_branch", "");
    display_branch->append_attribute(createXMLAttribute(&doc, "instance", instance_name));
    display_branch->append_attribute(createXMLAttribute(&doc, "signal_set", "USE_GLOBAL_TEMP"));
    display_branch->append_attribute(createXMLAttribute(&doc, "trigger", "USE_GLOBAL_TEMP"));
    display_tree->append_node(display_branch);
    
    xml_node<> *instance = createXMLNode(&doc, node_element, "instance", "");
    instance->append_attribute(createXMLAttribute(&doc, "entity_name", "sld_signaltap"));
    instance->append_attribute(createXMLAttribute(&doc, "is_auto_node", "yes"));
    instance->append_attribute(createXMLAttribute(&doc, "is_expanded", "true"));
    instance->append_attribute(createXMLAttribute(&doc, "name", instance_name));
    instance->append_attribute(createXMLAttribute(&doc, "source_file", "sld_signaltap.vhd"));
    
    xml_node<> *node_ip_info = createXMLNode(&doc, node_element, "node_ip_info", "");
    node_ip_info->append_attribute(createXMLAttribute(&doc, "instance_id", "0"));
    node_ip_info->append_attribute(createXMLAttribute(&doc, "mfg_id", "110"));
    node_ip_info->append_attribute(createXMLAttribute(&doc, "node_id", "0"));
    node_ip_info->append_attribute(createXMLAttribute(&doc, "version", "6"));
    instance->append_node(node_ip_info);
    
    xml_node<> *signal_set = createXMLNode(&doc, node_element, "signal_set", "");
    signal_set->append_attribute(createXMLAttribute(&doc, "is_expanded", "true"));
    signal_set->append_attribute(createXMLAttribute(&doc, "name", signal_set_name));
    
    xml_node<> *clock = createXMLNode(&doc, node_element, "clock", "");
    clock->append_attribute(createXMLAttribute(&doc, "name", "CLOCK_50"));
    clock->append_attribute(createXMLAttribute(&doc, "polarity", "posedge"));
    clock->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    signal_set->append_node(clock);    
    
    xml_node<> *config = createXMLNode(&doc, node_element, "config", "");
    config->append_attribute(createXMLAttribute(&doc, "ram_type", ram_type));
    config->append_attribute(createXMLAttribute(&doc, "reserved_data_nodes", "0"));
    config->append_attribute(createXMLAttribute(&doc, "reserved_storage_qualifier_nodes", "0"));
    config->append_attribute(createXMLAttribute(&doc, "reserved_trigger_nodes", "0"));
    config->append_attribute(createXMLAttribute(&doc, "sample_depth", sample_depth));
    config->append_attribute(createXMLAttribute(&doc, "trigger_in_enable", "no"));
    config->append_attribute(createXMLAttribute(&doc, "trigger_out_enable", "no"));
    signal_set->append_node(config);
    
    xml_node<> *top_entity = createXMLNode(&doc, node_element, "top_entity", "");
    signal_set->append_node(top_entity);        
    
    xml_node<> *signal_vec = createXMLNode(&doc, node_element, "signal_vec", "");
    
    xml_node<> *trigger_input_vec = createXMLNode(&doc, node_element, "trigger_input_vec", "");
    /*xml_node<> *trigger_wire = createXMLNode(&doc, node_element, "wire", "");
    trigger_wire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    trigger_wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    trigger_wire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    trigger_input_vec->append_node(trigger_wire);*/
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *trigger_counter = createXMLNode(&doc,node_element, "wire", "");
        trigger_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        trigger_counter->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
        trigger_counter->append_attribute(createXMLAttribute(&doc, "type", "register"));
        trigger_input_vec->append_node(trigger_counter);
    }
    
    
    signal_vec->append_node(trigger_input_vec);
    
    xml_node<> *data_input_vec = createXMLNode(&doc, node_element, "data_input_vec", "");
        
    for (int i = 0 ; i < allSignalsList.size(); i++) {
        std::string sig = allSignalsList[i];        
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
            if (node != NULL) {
                if (node->width == 1) {            
                    xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                    wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                    wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                    wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                    data_input_vec->append_node(wire);
                } else {
                    for (int idx = 0 ; idx < node->width; idx++)
                    {
                        xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                        wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                        wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                        wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                        data_input_vec->append_node(wire);
                    }
                }
            }
        }        
    }
    allFullNameSignalsList.clear();
        
    //adding finish signal to data vector manually...
    xml_node<> *finishWire = createXMLNode(&doc, node_element, "wire", "");
    finishWire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    finishWire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    finishWire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    data_input_vec->append_node(finishWire);
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *data_counter = createXMLNode(&doc,node_element, "wire", "");
        data_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        data_counter->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
        data_counter->append_attribute(createXMLAttribute(&doc, "type", "register"));
        data_input_vec->append_node(data_counter);
    }
    
    signal_vec->append_node(data_input_vec);
    
    xml_node<> *storage_qualifier_input_vec = createXMLNode(&doc, node_element, "storage_qualifier_input_vec", "");
    //adding trigger to the list
    xml_node<> *s_q_trigger_wire = createXMLNode(&doc, node_element, "wire", "");
    s_q_trigger_wire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));    
    s_q_trigger_wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    s_q_trigger_wire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    storage_qualifier_input_vec->append_node(s_q_trigger_wire);
    for (int i = 0 ; i < allSignalsList.size(); i++) {
        std::string sig = allSignalsList[i];
        
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
            if (node != NULL){
                if (node->width == 1) {            
                    xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                    wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                    wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                    wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                    storage_qualifier_input_vec->append_node(wire);
                } else {
                    for (int idx = 0 ; idx < node->width; idx++)
                    {
                        xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                        wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                        wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                        wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                        storage_qualifier_input_vec->append_node(wire);
                    }
                }
            }
        }
    }
    allFullNameSignalsList.clear();
    
    //adding finish signal to the storage qualifier....
    xml_node<> *finishSQWire = createXMLNode(&doc, node_element, "wire", "");
    finishSQWire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    finishSQWire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    finishSQWire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    storage_qualifier_input_vec->append_node(finishSQWire);
    
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *storage_qualifier_counter = createXMLNode(&doc,node_element, "wire", "");
        storage_qualifier_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        storage_qualifier_counter->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
        storage_qualifier_counter->append_attribute(createXMLAttribute(&doc, "type", "register"));
        storage_qualifier_input_vec->append_node(storage_qualifier_counter);
    }
    signal_vec->append_node(storage_qualifier_input_vec);
    
    signal_set->append_node(signal_vec);
    
    xml_node<> *presentation = createXMLNode(&doc, node_element, "presentation", "");
    xml_node<> *data_view = createXMLNode(&doc, node_element, "data_view", "");
    for (int i = 0 ; i < allSignalsList.size(); i++) {
      std::string sig = allSignalsList[i];
      
      for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
          FPGANode* node = signalsToNodesList[sig][j];
          if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
          if (node != NULL){
              if (node->width == 1) {            
                  xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                  net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                  net->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                  data_view->append_node(net);
              } else {
                  for (int idx = 0 ; idx < node->width; idx++)
                  {
                      xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                      net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                      net->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                      data_view->append_node(net);
                  }
              }
          }
      }  
    }
    allFullNameSignalsList.clear();
    //adding finish sig to vector...
    xml_node<> *finishSigNet = createXMLNode(&doc, node_element, "net", "");
    finishSigNet->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
    finishSigNet->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    data_view->append_node(finishSigNet);
    
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *data_view_counter = createXMLNode(&doc,node_element, "net", "");
        data_view_counter->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
        data_view_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        data_view->append_node(data_view_counter);
    }
    
    presentation->append_node(data_view);
    xml_node<> *setup_view = createXMLNode(&doc, node_element, "setup_view", "");
    for (int i = 0 ; i < allSignalsList.size(); i++) {
        std::string sig = allSignalsList[i];
        
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
            if (node != NULL){
                if (node->width == 1) {            
                    xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                    net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                    net->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                    setup_view->append_node(net);
                } else {
                    for (int idx = 0 ; idx < node->width; idx++)
                    {
                        xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                        net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                        net->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                        setup_view->append_node(net);
                    }
                }
            }
        }        
        allFullNameSignalsList.clear();
        
    }
    //adding finish sig...
    xml_node<> *finishSigSetupNet = createXMLNode(&doc, node_element, "net", "");
    finishSigSetupNet->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
    finishSigSetupNet->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    setup_view->append_node(finishSigSetupNet);
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *setup_view_counter = createXMLNode(&doc,node_element, "net", "");
        setup_view_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        setup_view->append_node(setup_view_counter);
    }
    
    xml_node<> *finish_for_setup = createXMLNode(&doc, node_element, "net", "");
    finish_for_setup->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    setup_view->append_node(finish_for_setup);
    
    presentation->append_node(setup_view);            
        
    signal_set->append_node(presentation);
    
    xml_node<> *trigger = createXMLNode(&doc, node_element, "trigger", "");
    trigger->append_attribute(createXMLAttribute(&doc, "attribute_mem_mode", "false"));
    trigger->append_attribute(createXMLAttribute(&doc, "gap_record", "true"));
    trigger->append_attribute(createXMLAttribute(&doc, "name", trigger_name));
    trigger->append_attribute(createXMLAttribute(&doc, "position", "post"));
    trigger->append_attribute(createXMLAttribute(&doc, "power_up_trigger_mode", "false"));
    trigger->append_attribute(createXMLAttribute(&doc, "record_data_gap", "true"));
    trigger->append_attribute(createXMLAttribute(&doc, "segment_size", "1"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_mode", "off"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_disabled", "no"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_port_is_pin", "true"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_port_name", "auto_stp_external_storage_qualifier"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_port_tap_mode", "classic"));
    trigger->append_attribute(createXMLAttribute(&doc, "trigger_in", "dont_care"));
    trigger->append_attribute(createXMLAttribute(&doc, "trigger_out", "active high"));
    trigger->append_attribute(createXMLAttribute(&doc, "trigger_type", "circular"));
    
    xml_node<> *power_up_trigger = createXMLNode(&doc, node_element, "power_up_trigger", "");
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "position", "pre"));
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_disabled", "no"));
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "trigger_in", "dont_care"));
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "trigger_out", "active high"));    
    trigger->append_node(power_up_trigger);
    
    xml_node<> *events = createXMLNode(&doc, node_element, "events", "");
    events->append_attribute(createXMLAttribute(&doc, "use_custom_flow_control", "no"));
    xml_node<> *level = createXMLNode(&doc, node_element, "level", "");
    std::string trigger_level_value;
    //int counterComparatorValue = triggerValue;
    int counterComparatorValue = triggerValue;
    int counters[32];
    for (int c = 31; c >= 0; c--)
    {
      int k = counterComparatorValue >> c;

      if (k & 1)
        counters[c]=1;
      else
        counters[c]=0;
      
      trigger_level_value += "'top:top_inst|counter[" + IntToString(c) +"]'==";
      if(counters[c]==1)
          trigger_level_value += "high";
      else
          trigger_level_value += "low";
      if (c==0)
          break;
      trigger_level_value += "&";
      trigger_level_value += "& ";
    }
    
    xml_node<> *level_value = createXMLNode(&doc, node_data, "", trigger_level_value);
    level->append_node(level_value);
    level->append_attribute(createXMLAttribute(&doc, "enabled", "yes"));
    level->append_attribute(createXMLAttribute(&doc, "name", "condition1"));
    level->append_attribute(createXMLAttribute(&doc, "type", "basic"));    
    //level->append_attribute(createXMLAttribute(&doc, "type", "advanced"));
    xml_node<> *power_up = createXMLNode(&doc, node_element, "power_up", "");
    power_up->append_attribute(createXMLAttribute(&doc, "enabled", "yes"));
    

        
    level->append_node(power_up);
    
    events->append_node(level);
    
    
    trigger->append_node(events);
    
    xml_node<> *storage_qualifier_events = createXMLNode(&doc, node_element, "storage_qualifier_events", "");
    xml_node<> *transitional = createXMLNode(&doc, node_element, "transitional", "");
    xml_node<> *pwr_up_transitional = createXMLNode(&doc, node_element, "pwr_up_transitional", "");
    transitional->append_node(pwr_up_transitional);
    
    storage_qualifier_events->append_node(transitional);
    
    int level_size = 3;
    for (int l = 0 ; l < level_size; l++) {
        xml_node<> *storage_qualifier_level = createXMLNode(&doc, node_element, "storage_qualifier_level", "");
        storage_qualifier_level->append_attribute(createXMLAttribute(&doc, "type", "basic"));
        xml_node<> *power_up = createXMLNode(&doc, node_element, "power_up", "");
        storage_qualifier_level->append_node(power_up);
        xml_node<> *op_node = createXMLNode(&doc, node_element, "op_node", "");
        storage_qualifier_level->append_node(op_node);
        storage_qualifier_events->append_node(storage_qualifier_level);
    }
    
    trigger->append_node(storage_qualifier_events);
    
    
    signal_set->append_node(trigger);
        
    
    
    instance->append_node(signal_set);
    
    //new change...
    xml_node<> *position_info = createXMLNode(&doc, node_element, "position_info", "");
    xml_node<> *single = createXMLNode(&doc, node_element, "single", "");
    single->append_attribute(createXMLAttribute(&doc, "attribute", "active tab"));
    single->append_attribute(createXMLAttribute(&doc, "value", "0"));
    position_info->append_node(single);
    
    instance->append_node(position_info);
 
    root->append_node(instance);
    
    xml_node<> *mnemonics = createXMLNode(&doc, node_element, "mnemonics", "");
    root->append_node(mnemonics);
    
    xml_node<> *global_info = createXMLNode(&doc, node_element, "global_info", "");
    
    xml_node<> *active_instance = createXMLNode(&doc, node_element, "single", "");
    active_instance->append_attribute(createXMLAttribute(&doc, "attribute", "active instance"));
    active_instance->append_attribute(createXMLAttribute(&doc, "value", "0"));
    global_info->append_node(active_instance);
    
    xml_node<> *config_widget_visible = createXMLNode(&doc, node_element, "single", "");
    config_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "config widget visible"));
    config_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(config_widget_visible);
    
    xml_node<> *data_log_widget_visible = createXMLNode(&doc, node_element, "single", "");
    data_log_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "data log widget visible"));
    data_log_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(data_log_widget_visible);
    
    xml_node<> *hierarchy_widget_visible = createXMLNode(&doc, node_element, "single", "");
    hierarchy_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "hierarchy widget visible"));
    hierarchy_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(hierarchy_widget_visible);
    
    xml_node<> *instance_widget_visible = createXMLNode(&doc, node_element, "single", "");
    instance_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "instance widget visible"));
    instance_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(instance_widget_visible);
    
    xml_node<> *jtag_widget_visible = createXMLNode(&doc, node_element, "single", "");
    jtag_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "jtag widget visible"));
    jtag_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(jtag_widget_visible);
    
    xml_node<> *frame_size = createXMLNode(&doc, node_element, "single", "");
    frame_size->append_attribute(createXMLAttribute(&doc, "attribute", "frame size"));
    frame_size->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(frame_size);
    
    xml_node<> *jtag_widget_size = createXMLNode(&doc, node_element, "single", "");
    jtag_widget_size->append_attribute(createXMLAttribute(&doc, "attribute", "jtag widget size"));
    jtag_widget_size->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(jtag_widget_size);
    
    root->append_node(global_info);
    
    xml_node<> *static_plugin_mnemonics = createXMLNode(&doc, node_element, "static_plugin_mnemonics", "");
    root->append_node(static_plugin_mnemonics);                    
    
    out << doc;
    
    out.flush();
    out.close();
}

void STPCreator::generateSTP(int triggerValue) {

    nodeFileLines.clear();
    //signalsToNodes.clear();
    signalsToNodesList.clear();
    
    std::ifstream in;
    in.open((workDir + nodeNamesFilename).c_str());
    
    while(in.good()) {
        std::string line;
        getline(in, line);
        nodeFileLines.push_back(line);
    }
    in.close();
    
    std::vector<std::string> allSignalsList;
    std::vector<std::string> allFullNameSignalsList;
        
    //adding all cur_state signals...
    std::vector<HWSignal*> curStateSignals = DA->getAllCurStateSignals();
    for (int i = 0 ; i < curStateSignals.size(); i++)
        onChipWatchSignals.insert(onChipWatchSignals.begin(), curStateSignals[i]);
    
    //adding all finish signals...
    std::vector<HWSignal*> finishSignals = DA->getAllFinishSignals();
    for (int i = 0 ; i < finishSignals.size(); i++)
        onChipWatchSignals.insert(onChipWatchSignals.begin(), finishSignals[i]);
    
    std::map<std::string, int> onChipWatchSignalsMap;
    for (int j = 0 ; j < onChipWatchSignals.size(); j++) {
        HWSignal* sig = onChipWatchSignals[j];
        std::string partialNodeName = sig->getFPGAPartialNodeName();
        int width = sig->width;
        onChipWatchSignalsMap[partialNodeName] = width;
    }
    
    addSignalsListToSTP(onChipWatchSignalsMap, allSignalsList);
    
    addSignalsListToSTP(selectedExtraSignals, allSignalsList);    
    
    addSignalsListToSTP(byDefaultAddedSignals, allSignalsList);    
    
    //add important signals to the stp file based on their appearance
    if (signalSelectionMode == AUTO_MODE_SIGNAL_SELECTION) {
        std::vector<HWSignal*> autoSignalsList;
        //std::vector<HWSignal*> autoSelectedSignals;
        std::map<std::string, int> autoSelectedSignals;
        for (int i = 0 ; i < Signals.size(); i++) {
            HWSignal *sig = Signals[i];            
            if (sig->name.find("memory_controller_") != std::string::npos ||
                    sig->name.find("LEGUP_F_") != std::string::npos ||
                    sig->states.size() == 0)
                continue;
            if (std::find(onChipWatchSignals.begin(), onChipWatchSignals.end(), sig) != onChipWatchSignals.end())
                continue;
            autoSignalsList.push_back(sig);
        }
        int windowStart = triggerValue - onChipDebugWindowSize;
        int windowEnd = triggerValue;
        for (int i = 0 ; i < autoSignalsList.size(); i++) {
            HWSignal *sig = autoSignalsList[i];            
            std::vector<int> signalCycles;
            for (int s = 0 ; s < sig->states.size(); s++) {
                State* state = sig->states[s];
                std::vector<int> cycles = statesToCycles[state->name];
                signalCycles.insert(signalCycles.end(), cycles.begin(), cycles.end());
            }
            bool signalObserved = false;
            for (int c = 0 ; c < signalCycles.size(); c++)
                if (signalCycles[c] >= windowStart && signalCycles[c] <= windowEnd) {
                    signalObserved = true;
                    break;
                }
            if (signalObserved)
                autoSelectedSignals[sig->getFPGAPartialNodeName()] = sig->width;
        }
        
        addSignalsListToSTP(autoSelectedSignals, allSignalsList);
    }
    
    
    std::ifstream deviceInfoStream;
    deviceInfoStream.open((workDir + deviceInfoFileName).c_str());
    
    std::string line;
    getline(deviceInfoStream, line);
    std::string jtag_chain = split(line, ';')[1];
    
    getline(deviceInfoStream, line);
    std::string jtag_device = split(line, ';')[1];    
    deviceInfoStream.close();
    
    //std::string jtag_chain = "USB-Blaster [2-1.2]";
    //std::string jtag_device = "@1: EP3C120/EP4CE115 (0x020F70DD)";
    std::string sof_file = "";
    std::string instance_name = "auto_signaltap_0";
    std::string signal_set_name = "signal_set_1";
    std::string trigger_name = "trigger_1";
    std::string ram_type = "M4K";
    std::string sample_depth = "128";
    
    std::ofstream out;
    out.open((workDir + stpFilename).c_str());    
    
    
    xml_document<> doc;    
            
    
    
    xml_node<> *root = createXMLNode(&doc, node_element, "session", "");        
    root->append_attribute(createXMLAttribute(&doc, "jtag_chain", jtag_chain));
    root->append_attribute(createXMLAttribute(&doc, "jtag_device", jtag_device));
    root->append_attribute(createXMLAttribute(&doc, "sof_file", sof_file));
    doc.append_node(root);
        
    
    xml_node<> *display_tree = createXMLNode(&doc, node_element, "display_tree", "");
    display_tree->append_attribute(createXMLAttribute(&doc, "gui_logging_enabled", "0"));
    root->append_node(display_tree);
    
    xml_node<> *display_branch = createXMLNode(&doc, node_element, "display_branch", "");
    display_branch->append_attribute(createXMLAttribute(&doc, "instance", instance_name));
    display_branch->append_attribute(createXMLAttribute(&doc, "signal_set", "USE_GLOBAL_TEMP"));
    display_branch->append_attribute(createXMLAttribute(&doc, "trigger", "USE_GLOBAL_TEMP"));
    display_tree->append_node(display_branch);
    
    xml_node<> *instance = createXMLNode(&doc, node_element, "instance", "");
    instance->append_attribute(createXMLAttribute(&doc, "entity_name", "sld_signaltap"));
    instance->append_attribute(createXMLAttribute(&doc, "is_auto_node", "yes"));
    instance->append_attribute(createXMLAttribute(&doc, "is_expanded", "true"));
    instance->append_attribute(createXMLAttribute(&doc, "name", instance_name));
    instance->append_attribute(createXMLAttribute(&doc, "source_file", "sld_signaltap.vhd"));
    
    xml_node<> *node_ip_info = createXMLNode(&doc, node_element, "node_ip_info", "");
    node_ip_info->append_attribute(createXMLAttribute(&doc, "instance_id", "0"));
    node_ip_info->append_attribute(createXMLAttribute(&doc, "mfg_id", "110"));
    node_ip_info->append_attribute(createXMLAttribute(&doc, "node_id", "0"));
    node_ip_info->append_attribute(createXMLAttribute(&doc, "version", "6"));
    instance->append_node(node_ip_info);
    
    xml_node<> *signal_set = createXMLNode(&doc, node_element, "signal_set", "");
    signal_set->append_attribute(createXMLAttribute(&doc, "is_expanded", "true"));
    signal_set->append_attribute(createXMLAttribute(&doc, "name", signal_set_name));
    
    xml_node<> *clock = createXMLNode(&doc, node_element, "clock", "");
    clock->append_attribute(createXMLAttribute(&doc, "name", "CLOCK_50"));
    clock->append_attribute(createXMLAttribute(&doc, "polarity", "posedge"));
    clock->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    signal_set->append_node(clock);    
    
    xml_node<> *config = createXMLNode(&doc, node_element, "config", "");
    config->append_attribute(createXMLAttribute(&doc, "ram_type", ram_type));
    config->append_attribute(createXMLAttribute(&doc, "reserved_data_nodes", "0"));
    config->append_attribute(createXMLAttribute(&doc, "reserved_storage_qualifier_nodes", "0"));
    config->append_attribute(createXMLAttribute(&doc, "reserved_trigger_nodes", "0"));
    config->append_attribute(createXMLAttribute(&doc, "sample_depth", sample_depth));
    config->append_attribute(createXMLAttribute(&doc, "trigger_in_enable", "no"));
    config->append_attribute(createXMLAttribute(&doc, "trigger_out_enable", "no"));
    signal_set->append_node(config);
    
    xml_node<> *top_entity = createXMLNode(&doc, node_element, "top_entity", "");
    signal_set->append_node(top_entity);        
    
    xml_node<> *signal_vec = createXMLNode(&doc, node_element, "signal_vec", "");
    
    xml_node<> *trigger_input_vec = createXMLNode(&doc, node_element, "trigger_input_vec", "");
    /*xml_node<> *trigger_wire = createXMLNode(&doc, node_element, "wire", "");
    trigger_wire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    trigger_wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    trigger_wire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    trigger_input_vec->append_node(trigger_wire);*/
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *trigger_counter = createXMLNode(&doc,node_element, "wire", "");
        trigger_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        trigger_counter->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
        trigger_counter->append_attribute(createXMLAttribute(&doc, "type", "register"));
        trigger_input_vec->append_node(trigger_counter);
    }
    
    
    signal_vec->append_node(trigger_input_vec);
    
    xml_node<> *data_input_vec = createXMLNode(&doc, node_element, "data_input_vec", "");   
    
    for (int i = 0 ; i < allSignalsList.size(); i++) {
        std::string sig = allSignalsList[i];        
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
            if (node != NULL) {
                if (node->width == 1) {            
                    xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                    wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                    wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                    wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                    data_input_vec->append_node(wire);
                } else {
                    for (int idx = 0 ; idx < node->width; idx++)
                    {
                        xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                        wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                        wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                        wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                        data_input_vec->append_node(wire);
                    }
                }
            }
        }        
    }
    allFullNameSignalsList.clear();
    
        
    //adding finish signal to data vector manually...
    xml_node<> *finishWire = createXMLNode(&doc, node_element, "wire", "");
    finishWire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    finishWire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    finishWire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    data_input_vec->append_node(finishWire);
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *data_counter = createXMLNode(&doc,node_element, "wire", "");
        data_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        data_counter->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
        data_counter->append_attribute(createXMLAttribute(&doc, "type", "register"));
        data_input_vec->append_node(data_counter);
    }
    
    signal_vec->append_node(data_input_vec);
    
    xml_node<> *storage_qualifier_input_vec = createXMLNode(&doc, node_element, "storage_qualifier_input_vec", "");
    //adding trigger to the list
    xml_node<> *s_q_trigger_wire = createXMLNode(&doc, node_element, "wire", "");
    s_q_trigger_wire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));    
    s_q_trigger_wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    s_q_trigger_wire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    storage_qualifier_input_vec->append_node(s_q_trigger_wire);
    for (int i = 0 ; i < allSignalsList.size(); i++) {
        std::string sig = allSignalsList[i];
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
        
        //FPGANode* node = signalsToNodes[sig];
            if (node != NULL){
                if (node->width == 1) {            
                    xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                    wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                    wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                    wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                    storage_qualifier_input_vec->append_node(wire);
                } else {
                    for (int idx = 0 ; idx < node->width; idx++)
                    {
                        xml_node<> *wire = createXMLNode(&doc, node_element, "wire", "");
                        wire->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                        wire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
                        wire->append_attribute(createXMLAttribute(&doc, "type", node->getTypeName()));
                        storage_qualifier_input_vec->append_node(wire);
                    }
                }
            }
        }
    }
    allFullNameSignalsList.clear();
    
    //adding finish signal to the storage qualifier....
    xml_node<> *finishSQWire = createXMLNode(&doc, node_element, "wire", "");
    finishSQWire->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    finishSQWire->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
    finishSQWire->append_attribute(createXMLAttribute(&doc, "type", "combinatorial"));
    storage_qualifier_input_vec->append_node(finishSQWire);
    
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *storage_qualifier_counter = createXMLNode(&doc,node_element, "wire", "");
        storage_qualifier_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        storage_qualifier_counter->append_attribute(createXMLAttribute(&doc, "tap_mode", "classic"));
        storage_qualifier_counter->append_attribute(createXMLAttribute(&doc, "type", "register"));
        storage_qualifier_input_vec->append_node(storage_qualifier_counter);
    }
    signal_vec->append_node(storage_qualifier_input_vec);
    
    signal_set->append_node(signal_vec);
    
    xml_node<> *presentation = createXMLNode(&doc, node_element, "presentation", "");
    xml_node<> *data_view = createXMLNode(&doc, node_element, "data_view", "");
    for (int i = 0 ; i < allSignalsList.size(); i++) {
      //std::string sig = allSignalsList[i];
      std::string sig = allSignalsList[i];
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
      //FPGANode* node = signalsToNodes[sig];
      if (node != NULL){
        if (node->width == 1) {            
            xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
            net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
            net->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
            data_view->append_node(net);
        } else {
            for (int idx = 0 ; idx < node->width; idx++)
            {
                xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                net->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                data_view->append_node(net);
            }
        }
      }
    }
}
    allFullNameSignalsList.clear();
    
    //adding finish sig to vector...
    xml_node<> *finishSigNet = createXMLNode(&doc, node_element, "net", "");
    finishSigNet->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
    finishSigNet->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    data_view->append_node(finishSigNet);
    
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *data_view_counter = createXMLNode(&doc,node_element, "net", "");
        data_view_counter->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
        data_view_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        data_view->append_node(data_view_counter);
    }
    
    presentation->append_node(data_view);
    xml_node<> *setup_view = createXMLNode(&doc, node_element, "setup_view", "");
    for (int i = 0 ; i < allSignalsList.size(); i++) {
        std::string sig = allSignalsList[i];
        for (int j = 0; j < signalsToNodesList[sig].size(); j++) {
            FPGANode* node = signalsToNodesList[sig][j];
            if (std::find(allFullNameSignalsList.begin(), allFullNameSignalsList.end(), node->fullName) != allFullNameSignalsList.end())
                continue;
            allFullNameSignalsList.push_back(node->fullName);
        //FPGANode* node = signalsToNodes[sig];
            if (node != NULL){
                if (node->width == 1) {            
                    xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                    net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                    net->append_attribute(createXMLAttribute(&doc, "name", node->fullName));
                    setup_view->append_node(net);
                } else {
                    for (int idx = 0 ; idx < node->width; idx++)
                    {
                        xml_node<> *net = createXMLNode(&doc, node_element, "net", "");
                        net->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
                        net->append_attribute(createXMLAttribute(&doc, "name", node->fullName + "[" + IntToString(idx) + "]"));
                        setup_view->append_node(net);
                    }
                }
            }
        }
    }
    allFullNameSignalsList.clear();
    
    //adding finish sig...
    xml_node<> *finishSigSetupNet = createXMLNode(&doc, node_element, "net", "");
    finishSigSetupNet->append_attribute(createXMLAttribute(&doc, "is_signal_inverted", "no"));
    finishSigSetupNet->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    setup_view->append_node(finishSigSetupNet);
    
    for (int i = 0 ; i < 32 ; i++)
    {
        std::string counter_name = "top:top_inst|counter["+ IntToString(i) +"]";
        xml_node<> *setup_view_counter = createXMLNode(&doc,node_element, "net", "");
        setup_view_counter->append_attribute(createXMLAttribute(&doc, "name", counter_name));
        setup_view->append_node(setup_view_counter);
    }
    
    xml_node<> *finish_for_setup = createXMLNode(&doc, node_element, "net", "");
    finish_for_setup->append_attribute(createXMLAttribute(&doc, "name", "top:top_inst|finish"));
    setup_view->append_node(finish_for_setup);
    
    presentation->append_node(setup_view);            
        
    signal_set->append_node(presentation);
    
    xml_node<> *trigger = createXMLNode(&doc, node_element, "trigger", "");
    trigger->append_attribute(createXMLAttribute(&doc, "attribute_mem_mode", "false"));
    trigger->append_attribute(createXMLAttribute(&doc, "gap_record", "true"));
    trigger->append_attribute(createXMLAttribute(&doc, "name", trigger_name));
    trigger->append_attribute(createXMLAttribute(&doc, "position", "post"));
    trigger->append_attribute(createXMLAttribute(&doc, "power_up_trigger_mode", "false"));
    trigger->append_attribute(createXMLAttribute(&doc, "record_data_gap", "true"));
    trigger->append_attribute(createXMLAttribute(&doc, "segment_size", "1"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_mode", "off"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_disabled", "no"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_port_is_pin", "true"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_port_name", "auto_stp_external_storage_qualifier"));
    trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_port_tap_mode", "classic"));
    trigger->append_attribute(createXMLAttribute(&doc, "trigger_in", "dont_care"));
    trigger->append_attribute(createXMLAttribute(&doc, "trigger_out", "active high"));
    trigger->append_attribute(createXMLAttribute(&doc, "trigger_type", "circular"));
    
    xml_node<> *power_up_trigger = createXMLNode(&doc, node_element, "power_up_trigger", "");
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "position", "pre"));
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "storage_qualifier_disabled", "no"));
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "trigger_in", "dont_care"));
    power_up_trigger->append_attribute(createXMLAttribute(&doc, "trigger_out", "active high"));    
    trigger->append_node(power_up_trigger);
    
    xml_node<> *events = createXMLNode(&doc, node_element, "events", "");
    events->append_attribute(createXMLAttribute(&doc, "use_custom_flow_control", "no"));
    xml_node<> *level = createXMLNode(&doc, node_element, "level", "");
    std::string trigger_level_value;
    //int counterComparatorValue = triggerValue;
    int counterComparatorValue = triggerValue;
    int counters[32];
    for (int c = 31; c >= 0; c--)
    {
      int k = counterComparatorValue >> c;

      if (k & 1)
        counters[c]=1;
      else
        counters[c]=0;
      
      trigger_level_value += "'top:top_inst|counter[" + IntToString(c) +"]'==";
      if(counters[c]==1)
          trigger_level_value += "high";
      else
          trigger_level_value += "low";
      if (c==0)
          break;
      trigger_level_value += "&";
      trigger_level_value += "& ";
    }
    
    xml_node<> *level_value = createXMLNode(&doc, node_data, "", trigger_level_value);
    level->append_node(level_value);
    level->append_attribute(createXMLAttribute(&doc, "enabled", "yes"));
    level->append_attribute(createXMLAttribute(&doc, "name", "condition1"));
    level->append_attribute(createXMLAttribute(&doc, "type", "basic"));    
    //level->append_attribute(createXMLAttribute(&doc, "type", "advanced"));
    xml_node<> *power_up = createXMLNode(&doc, node_element, "power_up", "");
    power_up->append_attribute(createXMLAttribute(&doc, "enabled", "yes"));
    
    
    //for Advanced trigger
    //createXMLNode()
    /*xml_node<> *power_up_expression = createXMLNode(&doc, node_element, "power_up_expression", "");
    std::string val = "'logical_0':(or('top:top_inst|finish','comparison_0':";
    val.append("(compare('bus_value_0':(variable(0)),==,'bus_0':({");
    val.append("'top:top_inst|counter[31]','top:top_inst|counter[30]','top:top_inst|counter[29]',");
    val.append("'top:top_inst|counter[28]','top:top_inst|counter[27]','top:top_inst|counter[26]',");
    val.append("'top:top_inst|counter[25]','top:top_inst|counter[24]','top:top_inst|counter[23]',");
    val.append("'top:top_inst|counter[22]','top:top_inst|counter[21]','top:top_inst|counter[20]',");
    val.append("'top:top_inst|counter[19]','top:top_inst|counter[18]','top:top_inst|counter[17]',");
    val.append("'top:top_inst|counter[16]','top:top_inst|counter[15]','top:top_inst|counter[14]',");
    val.append("'top:top_inst|counter[13]','top:top_inst|counter[12]','top:top_inst|counter[11]',");
    val.append("'top:top_inst|counter[10]','top:top_inst|counter[9]','top:top_inst|counter[8]',");
    val.append("'top:top_inst|counter[7]','top:top_inst|counter[6]','top:top_inst|counter[5]',");
    val.append("'top:top_inst|counter[4]','top:top_inst|counter[3]','top:top_inst|counter[2]',");
    val.append("'top:top_inst|counter[1]','top:top_inst|counter[0]'}),vc,uc))))");
    xml_node<> *power_up_expression_value = createXMLNode(&doc, node_cdata, "", val);
    power_up_expression->append_node(power_up_expression_value);
    power_up->append_node(power_up_expression);*/
        
    level->append_node(power_up);        
        
    events->append_node(level);
    
    
    trigger->append_node(events);
    
    xml_node<> *storage_qualifier_events = createXMLNode(&doc, node_element, "storage_qualifier_events", "");
    xml_node<> *transitional = createXMLNode(&doc, node_element, "transitional", "");
    xml_node<> *pwr_up_transitional = createXMLNode(&doc, node_element, "pwr_up_transitional", "");
    transitional->append_node(pwr_up_transitional);
    
    storage_qualifier_events->append_node(transitional);
    
    int level_size = 3;
    for (int l = 0 ; l < level_size; l++) {
        xml_node<> *storage_qualifier_level = createXMLNode(&doc, node_element, "storage_qualifier_level", "");
        storage_qualifier_level->append_attribute(createXMLAttribute(&doc, "type", "basic"));
        xml_node<> *power_up = createXMLNode(&doc, node_element, "power_up", "");
        storage_qualifier_level->append_node(power_up);
        xml_node<> *op_node = createXMLNode(&doc, node_element, "op_node", "");
        storage_qualifier_level->append_node(op_node);
        storage_qualifier_events->append_node(storage_qualifier_level);
    }
    
    trigger->append_node(storage_qualifier_events);
    
    
    signal_set->append_node(trigger);
        
    
    
    instance->append_node(signal_set);
    
    //new change...
    xml_node<> *position_info = createXMLNode(&doc, node_element, "position_info", "");
    xml_node<> *single = createXMLNode(&doc, node_element, "single", "");
    single->append_attribute(createXMLAttribute(&doc, "attribute", "active tab"));
    single->append_attribute(createXMLAttribute(&doc, "value", "0"));
    position_info->append_node(single);
    
    instance->append_node(position_info);
        
    //--------------------------
    root->append_node(instance);
    
    xml_node<> *mnemonics = createXMLNode(&doc, node_element, "mnemonics", "");
    root->append_node(mnemonics);
    
    xml_node<> *global_info = createXMLNode(&doc, node_element, "global_info", "");
    
    xml_node<> *active_instance = createXMLNode(&doc, node_element, "single", "");
    active_instance->append_attribute(createXMLAttribute(&doc, "attribute", "active instance"));
    active_instance->append_attribute(createXMLAttribute(&doc, "value", "0"));
    global_info->append_node(active_instance);
    
    xml_node<> *config_widget_visible = createXMLNode(&doc, node_element, "single", "");
    config_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "config widget visible"));
    config_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(config_widget_visible);
    
    xml_node<> *data_log_widget_visible = createXMLNode(&doc, node_element, "single", "");
    data_log_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "data log widget visible"));
    data_log_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(data_log_widget_visible);
    
    xml_node<> *hierarchy_widget_visible = createXMLNode(&doc, node_element, "single", "");
    hierarchy_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "hierarchy widget visible"));
    hierarchy_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(hierarchy_widget_visible);
    
    xml_node<> *instance_widget_visible = createXMLNode(&doc, node_element, "single", "");
    instance_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "instance widget visible"));
    instance_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(instance_widget_visible);
    
    xml_node<> *jtag_widget_visible = createXMLNode(&doc, node_element, "single", "");
    jtag_widget_visible->append_attribute(createXMLAttribute(&doc, "attribute", "jtag widget visible"));
    jtag_widget_visible->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(jtag_widget_visible);
    
    xml_node<> *frame_size = createXMLNode(&doc, node_element, "single", "");
    frame_size->append_attribute(createXMLAttribute(&doc, "attribute", "frame size"));
    frame_size->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(frame_size);
    
    xml_node<> *jtag_widget_size = createXMLNode(&doc, node_element, "single", "");
    jtag_widget_size->append_attribute(createXMLAttribute(&doc, "attribute", "jtag widget size"));
    jtag_widget_size->append_attribute(createXMLAttribute(&doc, "value", "1"));
    global_info->append_node(jtag_widget_size);
    
    root->append_node(global_info);
    
    xml_node<> *static_plugin_mnemonics = createXMLNode(&doc, node_element, "static_plugin_mnemonics", "");
    root->append_node(static_plugin_mnemonics);                    
    
    out << doc;
    
    out.flush();
    out.close();
}
