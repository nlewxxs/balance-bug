import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import MainMenu from './components/MainMenu.js';
import CreateSession from './components/CreateSession.js';
import LoadSession from './components/LoadSession.js';
import DisplayGraph from './components/DisplayGraph.js';
import logo from './logo.svg';
import './App.css';

function App() {
  return (
    <Router>
      <div className="App">
        <Routes>
          <Route exact path = "/" element={<MainMenu />}/>
          <Route exact path = "/Create" element={<CreateSession />}/>
          <Route exact path = "/Load" element={<LoadSession />}/>
          <Route exact path = "/Display/:id" element={<DisplayGraph />}/>
        </Routes>
        </div>
    </Router>
  );
}

export default App;
