import { BrowserRouter as Router, Route, Routes } from 'react-router-dom';
import MainMenu from './components/MainMenu.js';
import AddSession from './components/AddSession.js';
import LoadSession from './components/LoadSession.js';
import DisplayGraph from './components/DisplayGraph.js';
import './App.css';
// import logo from './logo.svg';

function App() {
  return (
    <Router>
      <div className="App">
        <Routes>
          <Route exact path = "/" element={<MainMenu />}/>
          <Route path = "/Add" element={<AddSession />}/>
          <Route path = "/Load" element={<LoadSession />}/>
          <Route path = "/Display/:id" element={<DisplayGraph />}/>
        </Routes>
        </div>
    </Router>
  );
}

export default App;
