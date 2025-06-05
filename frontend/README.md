# ULTSPY Web UI

This web interface allows operators to interact with the ULTSPY C2 backend. It is built with Vite, React, TypeScript and TailwindCSS. The UI components follow the shadcn/ui style guidelines.

## Setup

```bash
npm install
npm run dev
```

The development server proxies API requests to `http://localhost:5000`. Store your API key after logging in; requests include it via the `X-API-KEY` header.

## Project Structure

- `src/layout` contains the shared `Header`, `Sidebar` and `Layout` components.
- `src/pages` defines the routes:
  - **Dashboard** – landing page
  - **Agents** – lists `/admin/agents`
  - **Commands** – manage the queued commands per agent
  - **Logs** – placeholder for log viewing
  - **Settings** – placeholder for future settings

Navigation is handled via React Router DOM and styled with TailwindCSS.

## Commands Interface

Open the **Commands** page to inspect or modify the command queue for a specific agent.

1. Choose an agent from the dropdown. The queue is loaded from:
   ```
   GET /admin/commands/<uuid>
   -> { "commands": [{"command":"RECON","parameters":{},"timestamp":"..."}] }
   ```
2. Add a new command by selecting a command name and providing parameters as JSON. The request is sent to:
   ```
   POST /admin/command/<uuid>
   { "command": "RECON", "parameters": {}}
   ```
   A confirmation is shown on success.

Invalid JSON parameters are reported to the user. All requests include the stored API key via the `X-API-KEY` header.
