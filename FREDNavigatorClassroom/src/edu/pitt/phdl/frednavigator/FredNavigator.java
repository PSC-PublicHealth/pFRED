package edu.pitt.phdl.frednavigator;

// FRED Navigator
// Kirsten Meyling Taing

// Okay, so I'm going to try and sort this all out. I need to make some type of
// splash page. The home page.

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.Scanner;
import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

public class FredNavigator extends Application {
  
  public static final String pathToFRED = FredNavigator.getPathToFREDDirectory();
  
  private static String getPathToFREDDirectory()
  {    
    Scanner fileScanner = getFileScanner("FRED_Navigator_Settings");
    
    while(fileScanner.hasNext())
    {
      String input = fileScanner.next();
      if(input.contains("PATH="))
      {
        System.out.println("input.substring(5) = [" + input.substring(5) + "]");
        return input.substring(5);
      }
    }
    return null;
  }
  
  public static Scanner getFileScanner(String filename)
  {
    Scanner scanner = null;
    try
    {
      scanner = new Scanner(new java.io.File(filename));
    }
    catch(Exception e)
    {
      return null;
    }
    return scanner;
  }
  
  @Override
  public void start(Stage homepageStage) {
    
    FredNavigatorContext myContext = FredNavigatorContext.getInstance();
    
    try
    {
      // Each of the windows is a Stage, and the layout for the window is a Scene.
      Parent homepageLayout = FXMLLoader.load(getClass().getResource("homepage.fxml"));
      homepageStage = new Stage(StageStyle.UTILITY);
      homepageStage.setTitle("Welcome to FRED Navigator");
      Scene homepageScene = new Scene(homepageLayout);
      homepageStage.setScene(homepageScene);
      
      myContext.setFredNavigatorStage(homepageStage);
    }
    catch (IOException iOException)
    {
      Logger.getLogger(HomepageController.class.getName()).log(Level.SEVERE, null, iOException);
    }
    
    homepageStage.show();
  }
  
  // Main method should be ignored
  public static void main (String [] args) {
    launch(args);
  }
}
