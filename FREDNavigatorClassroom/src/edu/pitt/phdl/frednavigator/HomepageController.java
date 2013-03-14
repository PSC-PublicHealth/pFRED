package edu.pitt.phdl.frednavigator;

import java.io.IOException;
import java.net.URL;
import java.util.ResourceBundle;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.fxml.Initializable;
import javafx.scene.Parent;
import javafx.scene.Scene;

public class HomepageController implements Initializable {
  // left blank because I don't know what to put here yet.

  @FXML
  private void openTopic1Page(ActionEvent event) {
    
    try
    {
      Parent topic1Layout;
      topic1Layout = FXMLLoader.load(getClass().getResource("/edu/pitt/phdl/frednavigator/topic1/topic1.fxml"));
      Scene topic1Scene = new Scene(topic1Layout);
      FredNavigatorContext.getInstance().getFredNavigatorStage().setScene(topic1Scene);     
    }
    catch (IOException ex)
    {
      Logger.getLogger(HomepageController.class.getName()).log(Level.SEVERE, null, ex);
    }
                       
  }
  
  @FXML
  private void openTopic2Page(ActionEvent event) {
    
    try
    {
      Parent topic2Layout;
      topic2Layout = FXMLLoader.load(getClass().getResource("/edu/pitt/phdl/frednavigator/topic2/topic2.fxml"));
      Scene topic2Scene = new Scene(topic2Layout);
      FredNavigatorContext.getInstance().getFredNavigatorStage().setScene(topic2Scene);     
    }
    catch (IOException ex)
    {
      Logger.getLogger(HomepageController.class.getName()).log(Level.SEVERE, null, ex);
    }
                       
  }
  
  @Override
  public void initialize(URL url, ResourceBundle rb) {
      System.out.println("URL: " + url.toString());
  }  
}
