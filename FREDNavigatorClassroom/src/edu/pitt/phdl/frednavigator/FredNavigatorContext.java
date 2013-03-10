/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package edu.pitt.phdl.frednavigator;

import javafx.stage.Stage;

/**
 *
 * @author ddg5
 */
public class FredNavigatorContext {
  
  private final static FredNavigatorContext instance = new FredNavigatorContext();

  public static FredNavigatorContext getInstance()
  {
    return instance;
  }
  
  private Stage fredNavigatorStage = null;

  public Stage getFredNavigatorStage()
  {
    return fredNavigatorStage;
  }
  
  public void setFredNavigatorStage(Stage inStage)
  {
    this.fredNavigatorStage = inStage;
  }
}
